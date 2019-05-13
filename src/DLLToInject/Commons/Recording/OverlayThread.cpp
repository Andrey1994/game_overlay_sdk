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

#include "OverlayThread.h"
#include "../Overlay/OverlayMessage.h"
#include "Utility/Constants.h"
#include "Utility/StringUtils.h"
#include "Utility/MessageLog.h"
#include "Utility/ProcessHelper.h"
#include "RecordingState.h"


namespace GameOverlay {

    HWND g_windowHandle = NULL;

    OverlayThread::~OverlayThread ()
    {
        Stop ();
    }

    void OverlayThread::Stop ()
    {
        HANDLE thread = reinterpret_cast<HANDLE>(overlayThread_.native_handle ());
        if (thread)
        {
            const auto threadID = GetThreadId (thread);
            if (overlayThread_.joinable ())
            {
                this->quit_ = true;
                overlayThread_.join ();
            }
        }
    }

    void OverlayThread::Start ()
    {
        g_messageLog.LogInfo ("OverlayThread", "Start overlay thread ");
        quit_ = false;
        overlayThread_ = std::thread ([this] {this->ThreadProc (); });
    }

    void OverlayThread::ThreadProc ()
    {
        RecordingState::GetInstance ().Start ();
        HANDLE mapFile = OpenFileMapping (
            FILE_MAP_ALL_ACCESS,
            FALSE,
            TEXT ("Global\\GameOverlayMap")
        );
        if (mapFile == NULL)
        {
            g_messageLog.LogError ("OverlayThread", "Failed to open file mapping", GetLastError ());
        }
        g_messageLog.LogInfo ("OverlayThread", "Opened mapped file");
        while (!this->quit_)
        {
            if (mapFile)
            {
                char *buf = (char *)MapViewOfFile (
                    mapFile,
                    FILE_MAP_ALL_ACCESS,
                    0,
                    0,
                    4096
                );
                if (buf)
                {
                    g_messageLog.LogInfo ("OverlayThread", "Read from mapped file");
                    RecordingState::GetInstance ().SetOverlayMessage (buf);
                    UnmapViewOfFile (buf);
                }
                else
                {
                    g_messageLog.LogError ("OverlayThread", "Failed to read from mapped file");
                }
            }
            Sleep (500);
        }
    }

}
