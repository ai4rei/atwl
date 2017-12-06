#define INITGUID

#include <windows.h>
#include <shlobj.h>
#include <shobjidl.h>

// {DC4AB720-C354-421E-9D20-358A58DD0E28}
DEFINE_GUID(CLSID_ExampleShellFolder,
0xDC4AB720, 0xC354, 0x421E, 0x9D, 0x20, 0x35, 0x8A, 0x58, 0xDD, 0x0E, 0x28);

class CExampleShellFolder : public IShellFolder, public IPersistFolder
{
    ULONG m_ulLocks;

public:
    CExampleShellFolder();
    ~CExampleShellFolder();

    // IUnknown
    STDMETHODIMP QueryInterface(REFIID riid, LPVOID* lppOut);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

    // IShellFolder
    STDMETHODIMP ParseDisplayName(HWND hWndOwner, LPBC lpbcReserved, LPOLESTR lpszDisplayName, ULONG* lpulEaten, LPITEMIDLIST* lppidlOut, ULONG* lpulAttributes);
    STDMETHODIMP EnumObjects(HWND hWndOwner, DWORD dwFlags, LPENUMIDLIST* lppEnumIDListOut);
    STDMETHODIMP BindToObject(LPCITEMIDLIST lpidl, LPBC lpbcReserved, REFIID riid, LPVOID* lppOut);
    STDMETHODIMP BindToStorage(LPCITEMIDLIST lpidl, LPBC lpbcReserved, REFIID riid, LPVOID* lppOut);
    STDMETHODIMP CompareIDs(LPARAM lParam, LPCITEMIDLIST lpidl1, LPCITEMIDLIST lpidl2);
    STDMETHODIMP CreateViewObject(HWND hWndOwner, REFIID riid, LPVOID* lppOut);
    STDMETHODIMP GetAttributesOf(UINT uCount, LPCITEMIDLIST* lppidl, ULONG* lpulInOut);
    STDMETHODIMP GetUIObjectOf(HWND hWndOwner, UINT uCount, LPCITEMIDLIST* lppidl, REFIID riid, UINT* lpuInOut, LPVOID* lppOut);
    STDMETHODIMP GetDisplayNameOf(LPCITEMIDLIST lpidl, DWORD dwFlags, LPSTRRET lpsrName);
    STDMETHODIMP SetNameOf(HWND hWndOwner, LPCITEMIDLIST lpidl, LPCOLESTR lpszName, DWORD dwFlags, LPITEMIDLIST* lppidlOut);

    // IPersist
    STDMETHODIMP GetClassID(CLSID* pclsid);

    // IPersistFolder
    STDMETHODIMP Initialize(LPCITEMIDLIST lpidl);
};

class CExampleShellFolderClassFactory : public IClassFactory
{
public:
    // IUnknown
    STDMETHODIMP QueryInterface(REFIID riid, LPVOID* lppOut);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

    // IClassFactory
    STDMETHODIMP CreateInstance(IUnknown* lpUnkOuter, REFIID riid, LPVOID* lppOut);
    STDMETHODIMP LockServer(BOOL fLock);
};

static CExampleShellFolderClassFactory l_ClassFactory;  // keep a single global instance
static ULONG l_ulLocks = 0;

// CExampleShellFolder
//

CExampleShellFolder::CExampleShellFolder()
{
    m_ulLocks = 0UL;
}

CExampleShellFolder::~CExampleShellFolder()
{
}

STDMETHODIMP CExampleShellFolder::QueryInterface(REFIID riid, LPVOID* lppOut)
{
    if(lppOut)
    {
        if(IsEqualIID(riid, IID_IUnknown))
        {
            lppOut[0] = static_cast< IUnknown* >(static_cast< IShellFolder* >(this));
        }
        else
        if(IsEqualIID(riid, IID_IShellFolder))
        {
            lppOut[0] = static_cast< IShellFolder* >(this);
        }
        else
        if(IsEqualIID(riid, IID_IPersist))
        {
            lppOut[0] = static_cast< IPersist* >(this);
        }
        else
        if(IsEqualIID(riid, IID_IPersistFolder))
        {
            lppOut[0] = static_cast< IPersistFolder* >(this);
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

STDMETHODIMP_(ULONG) CExampleShellFolder::AddRef()
{
    return InterlockedIncrement((LPLONG)&m_ulLocks);
}

STDMETHODIMP_(ULONG) CExampleShellFolder::Release()
{
    ULONG ulResult = InterlockedIncrement((LPLONG)&m_ulLocks);

    if(!ulResult)
    {
        delete this;
    }

    return ulResult;
}

STDMETHODIMP CExampleShellFolder::ParseDisplayName(HWND hWndOwner, LPBC lpbcReserved, LPOLESTR lpszDisplayName, ULONG* lpulEaten, LPITEMIDLIST* lppidlOut, ULONG* lpulAttributes)
{
    return E_NOTIMPL;
}

STDMETHODIMP CExampleShellFolder::EnumObjects(HWND hWndOwner, DWORD dwFlags, LPENUMIDLIST* lppEnumIDListOut)
{
    lppEnumIDListOut[0] = NULL;

    return S_FALSE;  // no children
}

STDMETHODIMP CExampleShellFolder::BindToObject(LPCITEMIDLIST lpidl, LPBC lpbcReserved, REFIID riid, LPVOID* lppOut)
{
    return E_NOTIMPL;
}

STDMETHODIMP CExampleShellFolder::BindToStorage(LPCITEMIDLIST lpidl, LPBC lpbcReserved, REFIID riid, LPVOID* lppOut)
{
    return E_NOTIMPL;
}

STDMETHODIMP CExampleShellFolder::CompareIDs(LPARAM lParam, LPCITEMIDLIST lpidl1, LPCITEMIDLIST lpidl2)
{
    return E_NOTIMPL;
}

STDMETHODIMP CExampleShellFolder::CreateViewObject(HWND hWndOwner, REFIID riid, LPVOID* lppOut)
{
    return E_NOTIMPL;
}

STDMETHODIMP CExampleShellFolder::GetAttributesOf(UINT uCount, LPCITEMIDLIST* lppidl, ULONG* lpulInOut)
{
    return E_NOTIMPL;
}

STDMETHODIMP CExampleShellFolder::GetUIObjectOf(HWND hWndOwner, UINT uCount, LPCITEMIDLIST* lppidl, REFIID riid, UINT* lpuInOut, LPVOID* lppOut)
{
    return E_NOTIMPL;
}

STDMETHODIMP CExampleShellFolder::GetDisplayNameOf(LPCITEMIDLIST lpidl, DWORD dwFlags, LPSTRRET lpsrName)
{
    return E_NOTIMPL;
}

STDMETHODIMP CExampleShellFolder::SetNameOf(HWND hWndOwner, LPCITEMIDLIST lpidl, LPCOLESTR lpszName, DWORD dwFlags, LPITEMIDLIST* lppidlOut)
{
    return E_NOTIMPL;
}

STDMETHODIMP CExampleShellFolder::GetClassID(CLSID* pclsid)
{
    pclsid[0] = CLSID_ExampleShellFolder;

    return S_OK;
}

STDMETHODIMP CExampleShellFolder::Initialize(LPCITEMIDLIST lpidl)
{
    return S_OK;
}

// CExampleShellFolderClassFactory
//

STDMETHODIMP CExampleShellFolderClassFactory::QueryInterface(REFIID riid, LPVOID* lppOut)
{
    if(lppOut)
    {
        if(IsEqualIID(riid, IID_IUnknown))
        {
            lppOut[0] = static_cast< IUnknown* >(this);
        }
        else
        if(IsEqualIID(riid, IID_IClassFactory))
        {
            lppOut[0] = static_cast< IClassFactory* >(this);
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

STDMETHODIMP_(ULONG) CExampleShellFolderClassFactory::AddRef()
{
    return InterlockedIncrement((LPLONG)&l_ulLocks);
}

STDMETHODIMP_(ULONG) CExampleShellFolderClassFactory::Release()
{
    return InterlockedDecrement((LPLONG)&l_ulLocks);
}

STDMETHODIMP CExampleShellFolderClassFactory::CreateInstance(IUnknown* lpUnkOuter, REFIID riid, LPVOID* lppOut)
{
    if(lppOut)
    {
        if(lpUnkOuter)
        {
            return CLASS_E_NOAGGREGATION;
        }

        CExampleShellFolder* ExampleShellFolder = NULL;

        try
        {
            ExampleShellFolder = new CExampleShellFolder;
        }
        catch(...)
        {
            ;
        }

        if(ExampleShellFolder==NULL)
        {
            return E_OUTOFMEMORY;
        }

        HRESULT hr = ExampleShellFolder->QueryInterface(riid, lppOut);

        if(FAILED(hr))
        {
            delete ExampleShellFolder;
        }

        return hr;
    }

    return E_INVALIDARG;
}

STDMETHODIMP CExampleShellFolderClassFactory::LockServer(BOOL fLock)
{
    if(fLock)
    {
        InterlockedIncrement((LPLONG)&l_ulLocks);
    }
    else
    {
        InterlockedDecrement((LPLONG)&l_ulLocks);
    }

    return S_OK;
}

// Public
//

STDAPI DllCanUnloadNow()
{
    return l_ulLocks==0UL ? S_OK : S_FALSE;
}

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* lppOut)
{
    if(lppOut)
    {
        if(IsEqualCLSID(rclsid, CLSID_ExampleShellFolder))
        {
            return l_ClassFactory.QueryInterface(riid, lppOut);
        }

        lppOut[0] = NULL;

        return CLASS_E_CLASSNOTAVAILABLE;
    }

    return E_INVALIDARG;
}
