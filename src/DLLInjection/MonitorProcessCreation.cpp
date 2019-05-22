#include <string.h>

#include "MonitorProcessCreation.h"
#include "Monitor.h"

Monitor *monitor = NULL;

int SetLogLevel (int level)
{
    Monitor::SetLogLevel (level);
    return STATUS_OK;
}

int StartMonitor (char *processName, char *dllLoc)
{
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

int StopMonitor ()
{
    if (!monitor)
    {
        Monitor::monitorLogger->error ("process monitor is not running");
        return PROCESS_MONITOR_IS_NOT_RUNNING_ERROR;
    }
    int res = monitor->StopMonitor ();
    delete monitor;
    monitor = NULL;
    return res;
}

int GetPid (int *pid)
{
    if (!monitor)
    {
        Monitor::monitorLogger->error ("process monitor is not running");
        return PROCESS_MONITOR_IS_NOT_RUNNING_ERROR;
    }
    *pid = monitor->GetPid ();
    if (pid == 0)
        return TARGET_PROCESS_IS_NOT_CREATED_ERROR;
    return STATUS_OK;
}

int SendMessageToOverlay (char *message)
{
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
                monitor->StopMonitor ();
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