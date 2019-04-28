#include "Win32Handle.h"

Win32Handle::~Win32Handle ()
{
    Close ();
}

HANDLE Win32Handle::Get () const
{
    return handle_;
}

void Win32Handle::Close ()
{
    if (handle_ != INVALID_HANDLE_VALUE)
    {
        CloseHandle(handle_);
        handle_ = INVALID_HANDLE_VALUE;
    }
}
