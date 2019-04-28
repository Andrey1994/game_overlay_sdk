#ifndef EVENTSINK
#define EVENTSINK

#define _WIN32_DCOM

#include <comdef.h>
#include <Wbemidl.h>
#include <atlcomcli.h>
#include <atlstr.h>
#include <string.h>

#include "Monitor.h"

#pragma comment (lib, "wbemuuid.lib")

class EventSink : public IWbemObjectSink
{
    public:
        CComPtr<IWbemServices> pSvc;
        CComPtr<IWbemObjectSink> pStubSink;
        LONG mIRef;
        Monitor *monitor;

        EventSink (Monitor *mon) :mIRef (0), monitor (mon) {}
        ~EventSink () {}

        virtual ULONG STDMETHODCALLTYPE AddRef ()
        {
            return InterlockedIncrement (&mIRef);
        }

        virtual ULONG STDMETHODCALLTYPE Release ()
        {
            LONG IRef = InterlockedDecrement (&mIRef);
            if (IRef == 0)
                delete this;
            return IRef;
        }

        virtual HRESULT STDMETHODCALLTYPE QueryInterface (REFIID riid, void** ppv)
        {
            if (riid == IID_IUnknown || riid == IID_IWbemObjectSink)
            {
                *ppv = (IWbemObjectSink*)this;
                AddRef ();
                return WBEM_S_NO_ERROR;
            }
            else
                return E_NOINTERFACE;
        }

        virtual HRESULT STDMETHODCALLTYPE Indicate(
            LONG lObjectCount,
            IWbemClassObject __RPC_FAR *__RPC_FAR *apObjArray
        )
        {
            for (int i = 0; i < lObjectCount; i++)
            {
                _variant_t vtProp;

                HRESULT hr = apObjArray[i]->Get (_bstr_t(L"TargetInstance"), 0, &vtProp, 0, 0);
                IUnknown* str = vtProp;
                hr = str->QueryInterface (IID_IWbemClassObject, reinterpret_cast<void**>(&apObjArray[i]));
                if (SUCCEEDED (hr))
                {
                    _variant_t cn;
                    hr = apObjArray[i]->Get (L"Name", 0, &cn, NULL, NULL);
                    if (SUCCEEDED (hr))
                    {
                        char *pName = _com_util::ConvertBSTRToString (cn.bstrVal);
                        hr = apObjArray[i]->Get (L"ProcessId", 0, &cn, NULL, NULL);
                        if (SUCCEEDED (hr))
                        {
                            monitor->Callback (cn.intVal, pName);
                        }
                    }
                }
            }

            return WBEM_S_NO_ERROR;
        }

        virtual HRESULT STDMETHODCALLTYPE SetStatus (LONG IFlags, HRESULT hResult, BSTR strParam, IWbemClassObject __RPC_FAR *pObjParam)
        {
            return WBEM_S_NO_ERROR;
        }
};

#endif