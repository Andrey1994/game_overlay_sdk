#include <comdef.h>
#include <Wbemidl.h>
#include <atlcomcli.h>
#include <atlstr.h>
#include <string.h>
#include <accctrl.h>
#include <aclapi.h>

#include "MonitorProcessCreation.h"

#include "Win32Handle.h"
#include "DLLInjection.h"
#include "ProcessHelpers.h"
#include "EventSink.h"
#include "Monitor.h"
#include "suspend_threads.h"

#pragma comment (lib, "wbemuuid.lib")

#define MMAPSIZE 4096

std::shared_ptr<spdlog::logger> Monitor::monitorLogger = spdlog::stderr_logger_mt ("monitorLogger");

Monitor::Monitor (char *processName, char *dllLoc)
{
    strcpy ((char *)this->processName, processName);
    strcpy ((char *)this->dllLoc, dllLoc);
    this->thread = NULL;
    this->createEvent = NULL;
    this->stopEvent = NULL;
    this->mapFile = NULL;
    this->pid = 0;
}

Monitor::~Monitor ()
{
    this->StopMonitor ();
}

void Monitor::SetLogLevel (int level)
{
    int log_level = level;
    if (level > 6)
        log_level = 6;
    if (level < 0)
        log_level = 0;
    Monitor::monitorLogger->set_level (spdlog::level::level_enum (log_level));
    Monitor::monitorLogger->flush_on (spdlog::level::level_enum (log_level));
    DLLInjection::SetLogLevel (level);
}

int Monitor::StartMonitor ()
{
    Monitor::monitorLogger->trace ("start monitorring");
    this->mapFile = CreateFileMapping (
        INVALID_HANDLE_VALUE,
        NULL,
        PAGE_READWRITE,
        0,
        MMAPSIZE,
        TEXT ("Global\\GameOverlayMap")
    );
    if (this->mapFile == NULL)
    {
        Monitor::monitorLogger->error ("failed to create maped file Error {}", GetLastError ());
        return GENERAL_ERROR;
    }
    SetSecurityInfo (this->mapFile, SE_KERNEL_OBJECT,
        DACL_SECURITY_INFORMATION | PROTECTED_DACL_SECURITY_INFORMATION,
        NULL, NULL, NULL, NULL
    );

    this->createEvent = CreateEvent (NULL, TRUE, FALSE, TEXT ("createEvent"));
    this->stopEvent = CreateEvent (NULL, TRUE, FALSE, TEXT ("stopEvent"));
    if ((!this->createEvent) && (!this->stopEvent))
    {
        Monitor::monitorLogger->error ("Failed to create event");
        return GENERAL_ERROR;
    }
    this->thread = CreateThread (NULL, 0, Monitor::ThreadProc, this, 0, NULL);
    DWORD dwWait = WaitForSingleObject (createEvent, 3000);
    if (dwWait == WAIT_TIMEOUT)
    {
        Monitor::monitorLogger->error ("Failed to create monitorring thread");
        return GENERAL_ERROR;
    }
    return STATUS_OK;
}

int Monitor::StopMonitor ()
{
    Monitor::monitorLogger->trace ("stop monitorring");
    if (this->thread)
    {
        if (this->stopEvent)
        {
            SetEvent (this->stopEvent);
            WaitForSingleObject (this->thread, INFINITE);
        }
    }
    if (this->thread)
    {
        CloseHandle (this->thread);
        this->thread = NULL;
    }
    if (this->createEvent)
    {
        CloseHandle (this->createEvent);
        this->createEvent = NULL;
    }
    if (this->stopEvent)
    {
        CloseHandle (this->stopEvent);
        this->stopEvent = NULL;
    }
    if (this->mapFile)
    {
        CloseHandle (this->mapFile);
        this->mapFile = NULL;
    }
    this->pid = 0;
    return STATUS_OK;
}

int Monitor::GetPid ()
{
    return this->pid;
}

int Monitor::SendMessageToOverlay (char *message)
{
    if ((this->mapFile == NULL) || (this->pid == 0))
    {
        monitorLogger->error ("Overlay is not ready");
        return TARGET_PROCESS_IS_NOT_CREATED_ERROR;
    }
    monitorLogger->info ("sending message '{}' to {}", message, this->pid);
    char *buf = (char *) MapViewOfFile (
        this->mapFile,
        FILE_MAP_WRITE,
        0,
        0,
        MMAPSIZE
    );
    if (buf == NULL)
    {
        monitorLogger->error ("failed to create MapViewOfFile {}", GetLastError ());
        return GENERAL_ERROR;
    }
    CopyMemory ((PVOID)buf, message, (strlen (message) + 1) * sizeof(char));
    UnmapViewOfFile (buf);
    return STATUS_OK;
}

void Monitor::Callback (int pid, char *pName)
{
    if (strcmp (pName, (char *)this->processName) == 0)
    {
        int architecture = GetArchitecture (pid);
        Monitor::monitorLogger->info ("Target Process created, name {}, pid {}, architecture {}",
            pName, pid, architecture);
        DLLInjection dllInjection (pid, (char *)this->processName, architecture, (char *)this->dllLoc);
        SuspendAllThreads (pid);
        dllInjection.InjectDLL ();
        ResumeAllThreads (pid);
        this->pid = pid;
    }
    else
    {
        monitorLogger->trace ("Non Target Process created, name {}, pid {}", pName, pid);
    }
}

DWORD WINAPI Monitor::ThreadProc (LPVOID pMonitor)
{
    ((Monitor *) pMonitor)->WorkerThread ();
    return 0;
}

void Monitor::WorkerThread ()
{
    this->RegisterCreationCallback ();
    if (!SetEvent (this->createEvent))
    {
        Monitor::monitorLogger->error ("unable to set event");
        return;
    }
    WaitForSingleObject (this->stopEvent, INFINITE);
    Monitor::monitorLogger->trace ("monitoring thread was stopped");
}

bool Monitor::RegisterCreationCallback ()
{
    CComPtr<IWbemLocator> pLoc;
    CoInitializeEx (0, COINIT_MULTITHREADED);
    HRESULT hres = CoCreateInstance (CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER, IID_IWbemLocator, (LPVOID*)&pLoc);

    if (FAILED (hres))
    {
        Monitor::monitorLogger->error ("Failed to create IWbemLocator object error code {}", hres);
        return false;
    }
    CComPtr<EventSink> pSink (new EventSink (this));
    hres = pLoc->ConnectServer (_bstr_t (L"ROOT\\CIMV2"), NULL, NULL, 0, NULL, 0, 0, &pSink->pSvc);
    if (FAILED (hres))
    {
        Monitor::monitorLogger->error ("Failed to ConnectServer error code {}", hres);
        return false;
    }
    hres = CoSetProxyBlanket (pSink->pSvc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL, RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE);
    if (FAILED (hres))
    {
        Monitor::monitorLogger->error ("Failed to CoSetProxyBlanket error code {}", hres);
        return false;
    }

    CComPtr<IUnsecuredApartment> pUnsecApp;
    hres = CoCreateInstance (CLSID_UnsecuredApartment, NULL, CLSCTX_LOCAL_SERVER, IID_IUnsecuredApartment, (void**)&pUnsecApp);
    CComPtr<IUnknown> pStubUnk;
    pUnsecApp->CreateObjectStub (pSink, &pStubUnk);
    pStubUnk->QueryInterface (IID_IWbemObjectSink, (void**)&pSink->pStubSink);

    char buffer[512];
    sprintf_s (buffer, "SELECT * FROM __InstanceCreationEvent WITHIN 0.001 WHERE TargetInstance ISA 'Win32_Process'");

    hres = pSink->pSvc->ExecNotificationQueryAsync (_bstr_t ("WQL"), _bstr_t (buffer), WBEM_FLAG_SEND_STATUS, NULL, pSink->pStubSink);

    if (FAILED (hres))
    {
        Monitor::monitorLogger->error ("Failed to ExecNotificationQueryAsync error code {}", hres);
        return false;
    }
    return true;
}

// if failed 64bit is assumed!
int Monitor::GetArchitecture (int pid)
{
    const auto wow64FunctionAdress = GetProcAddress (GetModuleHandle ("kernel32"), "IsWow64Process");
    if (!wow64FunctionAdress)
    {
        Monitor::monitorLogger->error ("IsWow64Process function not found in kernel32");
        return 64;
    }

    BOOL wow64Process = true;
    Win32Handle processHandle = GetProcessHandleFromID (pid, PROCESS_QUERY_INFORMATION);
    if (!processHandle.Get ())
    {
        Monitor::monitorLogger->error ("can not get handle from pid");
        return 64;
    }
    if (!IsWow64Process (processHandle.Get (), &wow64Process))
    {
        Monitor::monitorLogger->error ("IsWow64Process failed, error {}", GetLastError ());
        return 64;
    }

    if (wow64Process)
        return 86;
    else
        return 64;
}
