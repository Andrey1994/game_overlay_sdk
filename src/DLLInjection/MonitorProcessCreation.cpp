#include <mutex>
#include <string.h>

#include "Monitor.h"
#include "MonitorProcessCreation.h"

Monitor *monitor = NULL;
std::mutex mutex;

int SetLogLevel (int level)
{
    std::lock_guard<std::mutex> lock (mutex);
    Monitor::SetLogLevel (level);
    return STATUS_OK;
}

int StartMonitor (char *processName, char *dllLoc)
{
    std::lock_guard<std::mutex> lock (mutex);
    if (monitor)
    {
        Monitor::monitorLogger->error ("process monitor already running");
        return PROCESS_MONITOR_ALREADY_RUNNING_ERROR;
    }
    monitor = new Monitor ();
    if (!monitor)
    {
        Monitor::monitorLogger->error ("failed to create monitor");
        return GENERAL_ERROR;
    }
    return monitor->StartMonitor (processName, dllLoc);
}

int RunProcess (char *exePath, char *args, char *dllLoc)
{
    std::lock_guard<std::mutex> lock (mutex);
    if (monitor)
    {
        Monitor::monitorLogger->error ("process monitor already running");
        return PROCESS_MONITOR_ALREADY_RUNNING_ERROR;
    }
    monitor = new Monitor ();
    if (!monitor)
    {
        Monitor::monitorLogger->error ("failed to create monitor");
        return GENERAL_ERROR;
    }
    return monitor->RunProcess (exePath, args, dllLoc);
}

int ReleaseResources ()
{
    std::lock_guard<std::mutex> lock (mutex);
    if (!monitor)
    {
        Monitor::monitorLogger->error ("process monitor is not running");
        return PROCESS_MONITOR_IS_NOT_RUNNING_ERROR;
    }
    int res = monitor->ReleaseResources ();
    delete monitor;
    monitor = NULL;
    return res;
}

int GetPid (int *pid)
{
    std::lock_guard<std::mutex> lock (mutex);
    if (!monitor)
    {
        Monitor::monitorLogger->error ("process monitor is not running");
        return PROCESS_MONITOR_IS_NOT_RUNNING_ERROR;
    }
    *pid = monitor->GetPid ();
    if (*pid == 0)
    {
        return TARGET_PROCESS_IS_NOT_CREATED_ERROR;
    }
    return STATUS_OK;
}

int SendMessageToOverlay (char *message)
{
    std::lock_guard<std::mutex> lock (mutex);
    if (!monitor)
    {
        Monitor::monitorLogger->error ("process monitor is not running");
        return PROCESS_MONITOR_IS_NOT_RUNNING_ERROR;
    }
    return monitor->SendMessageToOverlay (message);
}

BOOL WINAPI DllMain (HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved)
{
    switch (fdwReason)
    {
        case DLL_PROCESS_DETACH:
        {
            if (monitor)
            {
                monitor->ReleaseResources ();
                delete monitor;
                monitor = NULL;
            }
            break;
        }
        default:
            break;
    }
    return TRUE;
}