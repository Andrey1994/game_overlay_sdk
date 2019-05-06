//
// Copyright(c) 2016 Advanced Micro Devices, Inc. All rights reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//

#include <windows.h>
#include <string>
#include <tlhelp32.h>
#include <psapi.h>

#include "Win32Handle.h"
#include "DLLInjection.h"
#include "StringUtils.h"

std::shared_ptr<spdlog::logger> DLLInjection::injectLogger = spdlog::stderr_logger_mt ("recordLogger");
const std::string DLLInjection::dllNameX64 = "GameOverlay64.dll";
const std::string DLLInjection::dllNameX32 = "GameOverlay32.dll";

void DLLInjection::SetLogLevel (int level)
{
    int log_level = level;
    if (level > 6)
        log_level = 6;
    if (level < 0)
        log_level = 0;
    DLLInjection::injectLogger->set_level (spdlog::level::level_enum (log_level));
    DLLInjection::injectLogger->flush_on (spdlog::level::level_enum (log_level));
}

DLLInjection::DLLInjection (int pid, char *processName, int arch, char *dllPath)
{
    this->processHandle = NULL;
    this->remoteDLLAddress = NULL;
    this->pid = pid;
    this->dllPathSize = 0;
    this->processName = processName;
    std::string tmpPath (dllPath);
    this->arch = arch;
    tmpPath += "\\";
    if (arch == X86)
        tmpPath += DLLInjection::dllNameX32;
    else
        tmpPath += DLLInjection::dllNameX64;
    DLLInjection::injectLogger->info ("Using DLL {} for process {}", tmpPath, pid);
    this->dllPath = ConvertUTF8StringToUTF16String (tmpPath);
}

DLLInjection::~DLLInjection () {}

bool DLLInjection::InjectDLL ()
{
    DLLInjection::injectLogger->info ("starting injection to process {}", this->pid);

    if (!GetProcessHandle ())
    {
        return false;
    }

    if (!GetRemoteDLLAddress ())
    {
        return false;
    }

    if (!ExecuteLoadLibrary ())
    {
        return false;
    }

    DLLInjection::injectLogger->info ("injected");
    return true;
}

bool DLLInjection::FreeDLL ()
{
    DLLInjection::injectLogger->info ("starting free dll from process {}", this->pid);

    if (!GetProcessHandle ())
    {
        return false;
    }

    void* dllModule = GetRemoteDLLModule ();
    if (!dllModule)
    {
        return false;
    }

    if (!ExecuteFreeLibrary (dllModule)) {
        return false;
    }

    DLLInjection::injectLogger->info ("complete free dll from process {}", this->pid);

    if (this->processHandle)
    {
        if (this->remoteDLLAddress)
        {
            VirtualFreeEx (this->processHandle, this->remoteDLLAddress, 0, MEM_RELEASE);
            this->remoteDLLAddress = NULL;
        }
        CloseHandle (this->processHandle);
        this->processHandle = NULL;
    }

    return true;
}

bool DLLInjection::GetProcessHandle ()
{
    if (this->processHandle)
        return true;

    this->processHandle = OpenProcess (
        PROCESS_QUERY_INFORMATION | PROCESS_CREATE_THREAD | PROCESS_VM_OPERATION
        | PROCESS_VM_WRITE | PROCESS_VM_READ,
        FALSE, this->pid);
    if (!this->processHandle)
    {
        DLLInjection::injectLogger->error ("failed to open process {}", this->pid);
        return false;
    }

    DLLInjection::injectLogger->info ("got handle for process {}", this->pid);
    return true;
}

bool DLLInjection::GetRemoteDLLAddress ()
{
    // Because libAbsolutePath is used as c-string representation add size for the null terminator
    this->dllPathSize = (this->dllPath.size () + 1) * sizeof (this->dllPath[0]);
    this->remoteDLLAddress =
        VirtualAllocEx (this->processHandle, NULL, this->dllPathSize, MEM_COMMIT, PAGE_READWRITE);
    if (!this->remoteDLLAddress)
    {
        DLLInjection::injectLogger->error ("failed to allocate memory for dll in process {}, error {}", this->pid, GetLastError ());
        return false;
    }

    DLLInjection::injectLogger->info ("acquired remote dll address for {}", this->pid);
    return true;
}

bool DLLInjection::ExecuteLoadLibrary ()
{
    if (!WriteProcessMemory (this->processHandle, this->remoteDLLAddress,
                          this->dllPath.data (), this->dllPathSize, NULL))
    {
        DLLInjection::injectLogger->error ("failed to write process memory {}, error {}", this->pid, GetLastError ());
        return false;
    }
    DLLInjection::injectLogger->trace ("wrote process memory {}", this->pid);
    return ExecuteRemoteThread ("LoadLibraryW", this->remoteDLLAddress);
}

void* DLLInjection::GetRemoteDLLModule ()
{
    bool result = false;
    void* dllModule = nullptr;
    Win32Handle snapShot = CreateToolhelp32Snapshot (TH32CS_SNAPMODULE, this->pid);
    if (snapShot.Get ())
    {
        MODULEENTRY32 moduleEntry{};
        moduleEntry.dwSize = sizeof (moduleEntry);
        auto newModule = Module32First (snapShot.Get (), &moduleEntry);
        while (newModule)
        {
            DLLInjection::injectLogger->trace ("found {}", moduleEntry.szModule);
            if ((this->arch == X86) && (DLLInjection::dllNameX32.compare (moduleEntry.szModule) == 0) ||
                (this->arch == X64) && (DLLInjection::dllNameX64.compare (moduleEntry.szModule) == 0))
            {
                dllModule = moduleEntry.modBaseAddr;
                DLLInjection::injectLogger->info ("found remote dll module for process {}", this->pid);
                result = true;
                break;
            }
            newModule = Module32Next(snapShot.Get(), &moduleEntry);
        }
        if (!newModule)
        {
            DLLInjection::injectLogger->error ("unabke to find module in process {}", this->pid);
            result = false;
        }
    }
    else
    {
        DLLInjection::injectLogger->error ("createsnapshot failed for  {}, error {}", this->pid, GetLastError ());
        result = false;
    }
    return dllModule;
}

bool DLLInjection::ExecuteFreeLibrary (void* dllModule)
{
    return ExecuteRemoteThread ("FreeLibrary", dllModule);
}

bool DLLInjection::ExecuteRemoteThread (const std::string& functionName, void* functionArguments)
{
    const auto threadRoutine = reinterpret_cast<PTHREAD_START_ROUTINE>(
        GetProcAddress (GetModuleHandle (TEXT ("Kernel32")), functionName.c_str ()));
    DLLInjection::injectLogger->trace ("got threadRoutine for {}", this->pid);
    if (!threadRoutine)
    {
        DLLInjection::injectLogger->error ("GetProcAddress failed for  {}, error {}", this->pid, GetLastError ());
        return false;
    }

    Win32Handle remoteThread = CreateRemoteThread (this->processHandle, NULL, 0, threadRoutine, functionArguments, 0, NULL);
    if (!remoteThread.Get ())
    {
        DLLInjection::injectLogger->error ("CreateRemoteThread failed for  {}, error {}", this->pid, GetLastError ());
        return false;
    }
    DLLInjection::injectLogger->trace ("Creating remote thread for {}", this->pid);
    WaitForSingleObject (remoteThread.Get (), INFINITE);
    DLLInjection::injectLogger->trace ("Created remote thread for {}", this->pid);
    DWORD exitCode = 0;
    if (!GetExitCodeThread (remoteThread.Get (), &exitCode))
    {
        DLLInjection::injectLogger->error ("GetExitCodeThread failed for  {}, error {}", this->pid, GetLastError ());
    }
    if(!exitCode)
    {
        DLLInjection::injectLogger->error ("Remote thread failed, exit code is {} error is{}", exitCode, GetLastError ());
    }
    return true;
}
