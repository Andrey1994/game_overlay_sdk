#ifndef SUSPENDtHREADS
#define SUSPENDtHREADS

#include <windows.h>

bool SuspendAllThreads (DWORD pid);
bool ResumeAllThreads (DWORD pid);

#endif
