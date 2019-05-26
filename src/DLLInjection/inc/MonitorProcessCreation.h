#ifndef MONITORPROCESSCREATION
#define MONITORPROCESSCREATION

typedef enum
{
    STATUS_OK = 0,
    TARGET_PROCESS_IS_NOT_CREATED_ERROR,
    PROCESS_MONITOR_ALREADY_RUNNING_ERROR,
    PROCESS_MONITOR_IS_NOT_RUNNING_ERROR,
    GENERAL_ERROR,
    PATH_NOT_FOUND_ERROR,
    TARGET_PROCESS_WAS_TERMINATED_ERROR
} CustomExitCodes;

extern "C"
{
    __declspec(dllexport) int SetLogLevel (int level);
    __declspec(dllexport) int StartMonitor (char *processName, char *dllPath);
    __declspec(dllexport) int ReleaseResources ();
    __declspec(dllexport) int GetPid (int *pid);
    __declspec(dllexport) int SendMessageToOverlay (char *msg);
    __declspec(dllexport) int RunProcess (char *exePath, char *args, char *dllPath);
}

#endif
