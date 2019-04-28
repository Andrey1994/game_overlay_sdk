#include <string.h>
#include "MonitorProcessCreation.h"
#include "Monitor.h"

Monitor *monitor = NULL;

void SetLogLevel (int level)
{
    Monitor::SetLogLevel (level);
}

bool StartMonitor (char *processName, char *dllLoc)
{
    if (monitor)
    {
        Monitor::monitorLogger->error ("process monitor already running");
        return false;
    }
    monitor = new Monitor (processName, dllLoc);
    if (!monitor)
    {
        Monitor::monitorLogger->error ("failed to create monitor");
        return false;
    }
    return monitor->StartMonitor ();
}

bool StopMonitor ()
{
    if (!monitor)
    {
        Monitor::monitorLogger->error ("process monitor is not running");
        return false;
    }
    bool res = monitor->StopMonitor ();
    delete monitor;
    monitor = NULL;
    return res;
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