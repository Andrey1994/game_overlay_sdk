#include "ProcessHelpers.h"

HANDLE GetProcessHandleFromID (DWORD id, DWORD access)
{
    HANDLE handle = OpenProcess (access, FALSE, id);
    if (!handle)
    {
        return NULL;
    }
    return handle;
}