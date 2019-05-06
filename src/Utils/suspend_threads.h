#ifndef SUSPENDtHREADS
#define SUSPENDtHREADS

#include <windows.h>

bool suspend_all_threads (DWORD pid);
bool resume_all_threads (DWORD pid);

#endif
