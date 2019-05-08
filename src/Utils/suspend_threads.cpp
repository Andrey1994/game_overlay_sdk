#include "suspend_threads.h"

#include <tlhelp32.h>

bool SuspendAllThreads (DWORD pid)
{
    HANDLE hThreadSnap = INVALID_HANDLE_VALUE;
    THREADENTRY32 te32;

    hThreadSnap = CreateToolhelp32Snapshot (TH32CS_SNAPTHREAD, 0);
    if (hThreadSnap == INVALID_HANDLE_VALUE)
        return false;

    te32.dwSize = sizeof (THREADENTRY32);

    if (!Thread32First (hThreadSnap, &te32))
    {
        CloseHandle (hThreadSnap);
        return false;
    }

    do
    {
        if (te32.th32OwnerProcessID == pid)
        {
            HANDLE hThread = OpenThread (THREAD_SUSPEND_RESUME, FALSE, te32.th32ThreadID);
            SuspendThread (hThread);
            CloseHandle (hThread);
        }
    } while (Thread32Next (hThreadSnap, &te32));

    CloseHandle (hThreadSnap);
    return true;
}

bool ResumeAllThreads (DWORD pid)
{
    HANDLE hThreadSnap = INVALID_HANDLE_VALUE;
    THREADENTRY32 te32;

    hThreadSnap = CreateToolhelp32Snapshot (TH32CS_SNAPTHREAD, 0);
    if (hThreadSnap == INVALID_HANDLE_VALUE)
        return false;

    te32.dwSize = sizeof (THREADENTRY32);

    if (!Thread32First (hThreadSnap, &te32))
    {
        CloseHandle (hThreadSnap);
        return false;
    }

    do
    {
        if (te32.th32OwnerProcessID == pid)
        {
            HANDLE hThread = OpenThread (THREAD_SUSPEND_RESUME, FALSE, te32.th32ThreadID);
            ResumeThread (hThread);
            CloseHandle (hThread);
        }
    } while (Thread32Next (hThreadSnap, &te32));

    CloseHandle (hThreadSnap);
    return true;
}
