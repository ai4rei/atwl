#define STRICT_TYPED_ITEMIDS

#include <windows.h>
#include <shlobj.h>

#include "simplepidl.h"

class CFileSystemBindData : public IFileSystemBindData
{
private:
    ULONG m_ulLocks;

    WIN32_FIND_DATAW m_Wfd;

protected:
    CFileSystemBindData()
    {
        m_ulLocks = 1U;

        ZeroMemory(&m_Wfd, sizeof(m_Wfd));
    }
    ~CFileSystemBindData()
    {// this makes sure that you cannot delete it other than by
     // releasing it
    }

public:
    // IUnknown
    STDMETHODIMP QueryInterface(REFIID riid, LPVOID* lppOut)
    {
        if(lppOut)
        {
            if(IsEqualIID(riid, IID_IUnknown))
            {
                lppOut[0] = static_cast< IUnknown* >(this);
            }
            else
            if(IsEqualIID(riid, IID_IFileSystemBindData))
            {
                lppOut[0] = static_cast< IFileSystemBindData* >(this);
            }
            else
            {
                lppOut[0] = NULL;

                return E_NOINTERFACE;
            }

            AddRef();

            return S_OK;
        }

        return E_INVALIDARG;
    }

    STDMETHODIMP_(ULONG) AddRef()
    {
        return InterlockedIncrement((LPLONG)&m_ulLocks);
    }

    STDMETHODIMP_(ULONG) Release()
    {
        ULONG ulResult = InterlockedIncrement((LPLONG)&m_ulLocks);

        if(!ulResult)
        {
            delete this;
        }

        return ulResult;
    }

    // IFileSystemBindData
    STDMETHODIMP SetFindData(const WIN32_FIND_DATAW* lpWfd)
    {
        m_Wfd = lpWfd[0];

        return S_OK;
    }

    STDMETHODIMP GetFindData(WIN32_FIND_DATAW* lpWfd)
    {
        lpWfd[0] = m_Wfd;

        return S_OK;
    }

    // CFileSystemBindData
    static STDMETHODIMP Create(REFIID riid, LPVOID* lppOut)
    {
        CFileSystemBindData* lpFsb = new CFileSystemBindData;
        HRESULT hr;

        hr = lpFsb->QueryInterface(riid, lppOut);

        lpFsb->Release();

        return hr;
    }
};

STDAPI CreateSimpleIDListFromPath(HWND hWnd, LPCWSTR lpszPath, const WIN32_FIND_DATAW* lpWfd, PIDLIST_ABSOLUTE* lppidl)
{
    HRESULT hr;

    if(lpszPath && lppidl)
    {
        IFileSystemBindData* lpFsb = NULL;

        hr = CFileSystemBindData::Create(IID_PPV_ARGS(&lpFsb));

        if(!FAILED(hr))
        {
            IBindCtx* lpBc = NULL;

            if(lpWfd)
            {
                lpFsb->SetFindData(lpWfd);
            }

            hr = CreateBindCtx(0, &lpBc);

            if(!FAILED(hr))
            {
                hr = lpBc->RegisterObjectParam(STR_FILE_SYS_BIND_DATA, lpFsb);  // L"File System Bind Data"

                if(!FAILED(hr))
                {
                    IShellFolder* lpDesktop = NULL;

                    hr = SHGetDesktopFolder(&lpDesktop);

                    if(!FAILED(hr))
                    {
                        hr = lpDesktop->ParseDisplayName(hWnd, lpBc, const_cast< LPOLESTR >(lpszPath), NULL, (PIDLIST_RELATIVE*)lppidl, NULL);
                        lpDesktop->Release();
                    }
                }

                lpBc->Release();
            }

            lpFsb->Release();
        }
    }
    else
    {
        hr = E_INVALIDARG;
    }

    return hr;
}
