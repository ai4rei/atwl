#define INITGUID
#define STRICT_TYPED_ITEMIDS

#include <windows.h>
#include <shlobj.h>
#include <shobjidl.h>

// {1A67366A-3641-4575-BCFF-8D574C6F68F0}
DEFINE_GUID(CLSID_ExampleShellFolder2,
0x1A67366A, 0x3641, 0x4575, 0xBC, 0xFF, 0x8D, 0x57, 0x4C, 0x6F, 0x68, 0xF0);

class CExampleShellFolder2 : public IShellFolder2, IPersistFolder2  // No IShellDetails, because systems that need it do not have system-provided IShellView
{
    ULONG m_ulLocks;

public:
    CExampleShellFolder2();
    ~CExampleShellFolder2();

    // IUnknown
    STDMETHODIMP QueryInterface(REFIID riid, LPVOID* lppOut);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

    // IShellFolder
    STDMETHODIMP ParseDisplayName(HWND hWndOwner, LPBC lpbcReserved, LPOLESTR lpszDisplayName, ULONG* lpulEaten, PIDLIST_RELATIVE* lppidlOut, ULONG* lpulAttributes);
    STDMETHODIMP EnumObjects(HWND hWndOwner, DWORD dwFlags, LPENUMIDLIST* lppEnumIDListOut);
    STDMETHODIMP BindToObject(PCUIDLIST_RELATIVE lpidl, LPBC lpbcReserved, REFIID riid, LPVOID* lppOut);
    STDMETHODIMP BindToStorage(PCUIDLIST_RELATIVE lpidl, LPBC lpbcReserved, REFIID riid, LPVOID* lppOut);
    STDMETHODIMP CompareIDs(LPARAM lParam, PCUIDLIST_RELATIVE lpidl1, PCUIDLIST_RELATIVE lpidl2);
    STDMETHODIMP CreateViewObject(HWND hWndOwner, REFIID riid, LPVOID* lppOut);
    STDMETHODIMP GetAttributesOf(UINT uCount, PCUITEMID_CHILD_ARRAY lppidl, SFGAOF* lpulInOut);
    STDMETHODIMP GetUIObjectOf(HWND hWndOwner, UINT uCount, PCUITEMID_CHILD_ARRAY lppidl, REFIID riid, UINT* lpuInOut, LPVOID* lppOut);
    STDMETHODIMP GetDisplayNameOf(PCUITEMID_CHILD lpidl, SHGDNF dwFlags, LPSTRRET lpsrName);
    STDMETHODIMP SetNameOf(HWND hWndOwner, PCUITEMID_CHILD lpidl, LPCOLESTR lpszName, SHGDNF dwFlags, PITEMID_CHILD* lppidlOut);

    // IShellFolder2
    STDMETHODIMP EnumSearches(IEnumExtraSearch** lppEnum);
    STDMETHODIMP GetDefaultColumn(DWORD dwReserved, ULONG* lpulSort, ULONG* lpulDisplay);
    STDMETHODIMP GetDefaultColumnState(UINT uColumn, SHCOLSTATEF* lpcsFlags);
    STDMETHODIMP GetDefaultSearchGUID(GUID* lpGuid);
    STDMETHODIMP GetDetailsEx(PCUITEMID_CHILD lpidl, const SHCOLUMNID* lpScid, VARIANT* lpVarOut);
    STDMETHODIMP GetDetailsOf(PCUITEMID_CHILD lpidl, UINT uColumn, SHELLDETAILS* lpSd);
    STDMETHODIMP MapColumnToSCID(UINT uColumn, SHCOLUMNID* lpScid);

    // IPersist
    STDMETHODIMP GetClassID(CLSID* pclsid);

    // IPersistFolder
    STDMETHODIMP Initialize(PCIDLIST_ABSOLUTE lpidl);

    // IPersistFolder2
    STDMETHODIMP GetCurFolder(PIDLIST_ABSOLUTE* lppidl);
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

// CExampleShellFolder2
//

CExampleShellFolder2::CExampleShellFolder2()
{
    m_ulLocks = 0UL;
}

CExampleShellFolder2::~CExampleShellFolder2()
{
}

STDMETHODIMP CExampleShellFolder2::QueryInterface(REFIID riid, LPVOID* lppOut)
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
        if(IsEqualIID(riid, IID_IShellFolder2))
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
        if(IsEqualIID(riid, IID_IPersistFolder2))
        {
            lppOut[0] = static_cast< IPersistFolder2* >(this);
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

STDMETHODIMP_(ULONG) CExampleShellFolder2::AddRef()
{
    return InterlockedIncrement((LPLONG)&m_ulLocks);
}

STDMETHODIMP_(ULONG) CExampleShellFolder2::Release()
{
    ULONG ulResult = InterlockedIncrement((LPLONG)&m_ulLocks);

    if(!ulResult)
    {
        delete this;
    }

    return ulResult;
}

STDMETHODIMP CExampleShellFolder2::ParseDisplayName(HWND hWndOwner, LPBC lpbcReserved, LPOLESTR lpszDisplayName, ULONG* lpulEaten, PIDLIST_RELATIVE* lppidlOut, ULONG* lpulAttributes)
{
    return E_NOTIMPL;
}

STDMETHODIMP CExampleShellFolder2::EnumObjects(HWND hWndOwner, DWORD dwFlags, LPENUMIDLIST* lppEnumIDListOut)
{
    lppEnumIDListOut[0] = NULL;

    return S_FALSE;  // no children
}

STDMETHODIMP CExampleShellFolder2::BindToObject(PCUIDLIST_RELATIVE lpidl, LPBC lpbcReserved, REFIID riid, LPVOID* lppOut)
{
    return E_NOTIMPL;
}

STDMETHODIMP CExampleShellFolder2::BindToStorage(PCUIDLIST_RELATIVE lpidl, LPBC lpbcReserved, REFIID riid, LPVOID* lppOut)
{
    return E_NOTIMPL;
}

STDMETHODIMP CExampleShellFolder2::CompareIDs(LPARAM lParam, PCUIDLIST_RELATIVE lpidl1, PCUIDLIST_RELATIVE lpidl2)
{
    return E_NOTIMPL;
}

STDMETHODIMP CExampleShellFolder2::CreateViewObject(HWND hWndOwner, REFIID riid, LPVOID* lppOut)
{
    IShellView* lpShellView = NULL;
    SFV_CREATE Sfv = { sizeof(Sfv) };
    HRESULT hr;

    Sfv.pshf = static_cast< IShellFolder* >(this);

    hr = SHCreateShellFolderView(&Sfv, &lpShellView);

    if(!FAILED(hr))
    {
        hr = lpShellView->QueryInterface(riid, lppOut);

        lpShellView->Release();
    }

    return hr;
}

STDMETHODIMP CExampleShellFolder2::GetAttributesOf(UINT uCount, PCUITEMID_CHILD_ARRAY lppidl, SFGAOF* lpulInOut)
{
    return E_NOTIMPL;
}

STDMETHODIMP CExampleShellFolder2::GetUIObjectOf(HWND hWndOwner, UINT uCount, PCUITEMID_CHILD_ARRAY lppidl, REFIID riid, UINT* lpuInOut, LPVOID* lppOut)
{
    return E_NOTIMPL;
}

STDMETHODIMP CExampleShellFolder2::GetDisplayNameOf(PCUITEMID_CHILD lpidl, SHGDNF dwFlags, LPSTRRET lpsrName)
{
    return E_NOTIMPL;
}

STDMETHODIMP CExampleShellFolder2::SetNameOf(HWND hWndOwner, PCUITEMID_CHILD lpidl, LPCOLESTR lpszName, SHGDNF dwFlags, PITEMID_CHILD* lppidlOut)
{
    return E_NOTIMPL;
}

STDMETHODIMP CExampleShellFolder2::EnumSearches(IEnumExtraSearch** lppEnum)
{
    return E_NOTIMPL;
}

STDMETHODIMP CExampleShellFolder2::GetDefaultColumn(DWORD dwReserved, ULONG* lpulSort, ULONG* lpulDisplay)
{
    return E_NOTIMPL;
}

STDMETHODIMP CExampleShellFolder2::GetDefaultColumnState(UINT uColumn, SHCOLSTATEF* lpcsFlags)
{
    return E_NOTIMPL;
}

STDMETHODIMP CExampleShellFolder2::GetDefaultSearchGUID(GUID* lpGuid)
{
    return E_NOTIMPL;
}

STDMETHODIMP CExampleShellFolder2::GetDetailsEx(PCUITEMID_CHILD lpidl, const SHCOLUMNID* lpScid, VARIANT* lpVarOut)
{
    return E_NOTIMPL;
}

STDMETHODIMP CExampleShellFolder2::GetDetailsOf(PCUITEMID_CHILD lpidl, UINT uColumn, SHELLDETAILS* lpSd)
{
    return E_NOTIMPL;
}

STDMETHODIMP CExampleShellFolder2::MapColumnToSCID(UINT uColumn, SHCOLUMNID* lpScid)
{
    return E_NOTIMPL;
}

STDMETHODIMP CExampleShellFolder2::GetClassID(CLSID* pclsid)
{
    pclsid[0] = CLSID_ExampleShellFolder2;

    return S_OK;
}

STDMETHODIMP CExampleShellFolder2::Initialize(PCIDLIST_ABSOLUTE lpidl)
{
    return S_OK;
}

STDMETHODIMP CExampleShellFolder2::GetCurFolder(PIDLIST_ABSOLUTE* lppidl)
{
    return E_NOTIMPL;
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

        CExampleShellFolder2* ExampleShellFolder = NULL;

        try
        {
            ExampleShellFolder = new CExampleShellFolder2;
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
        if(IsEqualCLSID(rclsid, CLSID_ExampleShellFolder2))
        {
            return l_ClassFactory.QueryInterface(riid, lppOut);
        }

        lppOut[0] = NULL;

        return CLASS_E_CLASSNOTAVAILABLE;
    }

    return E_INVALIDARG;
}
