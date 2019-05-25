#ifndef WIN32HANDLE
#define WIN32HANDLE

#include <windows.h>

class Win32Handle
{
    void Close ();
    HANDLE handle_;

public:
    Win32Handle (HANDLE handle) : handle_ (handle)
    {
    }
    ~Win32Handle ();

    Win32Handle (const Win32Handle &other) = delete;
    Win32Handle &operator= (const Win32Handle &other) = delete;
    Win32Handle (Win32Handle &&other) = delete;

    HANDLE Get () const;
};

#endif