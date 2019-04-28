#ifndef MONITORPROCESSCREATION
#define MONITORPROCESSCREATION

extern "C" {
    __declspec(dllexport) void SetLogLevel (int level);
    __declspec(dllexport) bool StartMonitor (char *processName, char *dllPath);
    __declspec(dllexport) bool StopMonitor ();
}

#endif
