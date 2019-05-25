//
// Copyright(c) 2016 Advanced Micro Devices, Inc. All rights reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//

#ifndef DLLINJECTION
#define DLLINJECTION

#include <string>
#include <windows.h>

#include <logger\spdlog.h>

typedef enum
{
    X86 = 86,
    X64 = 64,
    UNDEF = 0
} Arch;

class DLLInjection
{
    public:
        static std::shared_ptr<spdlog::logger> injectLogger;
        static void SetLogLevel (int level);

        DLLInjection (int pid, int arch, char *dllPath);
        ~DLLInjection ();

        bool InjectDLL ();
        bool FreeDLL ();
        HANDLE GetTargetProcessHandle ();

        static const std::string dllNameX64;
        static const std::string dllNameX32;

    private:
        int pid;
        int arch;
        std::wstring dllPath;

        size_t dllPathSize;
        HANDLE processHandle;
        void *remoteDLLAddress;

        bool GetProcessHandle ();
        bool GetRemoteDLLAddress ();
        bool ExecuteLoadLibrary ();
        void *GetRemoteDLLModule ();
        bool ExecuteFreeLibrary (void *dllModule);
        bool ExecuteRemoteThread (const std::string &functionName, void *functionArguments);
};

#endif
