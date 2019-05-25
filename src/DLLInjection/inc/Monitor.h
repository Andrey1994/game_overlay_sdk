#ifndef MONITOR
#define MONITOR

#define _WIN32_DCOM

#include <windows.h>
#include <logger\spdlog.h>

class Monitor
{
    public:
        static std::shared_ptr<spdlog::logger> monitorLogger;
        static void SetLogLevel (int level);

        Monitor ();
        ~Monitor ();

        int StartMonitor (char *processName, char *dllLoc);
        int RunProcess (char *exePath, char *args, char *dllLoc);
        int StopMonitor ();
        void Callback (int pid, char *pName);
        int GetPid ();
        int SendMessageToOverlay (char *message);

    private:
        volatile HANDLE thread;
        volatile HANDLE createEvent;
        volatile HANDLE stopEvent;
        volatile HANDLE mapFile;
        volatile char processName[1024];
        volatile char dllLoc[1024];
        volatile int pid;
        volatile HANDLE processHandle;

        bool RegisterCreationCallback ();
        static DWORD WINAPI ThreadProc (LPVOID lpParameter);
        void WorkerThread ();
        int GetArchitecture (int pid);
        HANDLE GetProcessHandleFromID (DWORD id, DWORD access);
        int CreateDesktopProcess (char *path, char *cmdArgs);
        int CreateFileMap ();
        bool CheckTargetProcessAlive ();
};

#endif
