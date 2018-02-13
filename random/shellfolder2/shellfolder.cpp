#define INITGUID
#define STRICT_TYPED_ITEMIDS

#include <windows.h>
#include <shlobj.h>
#include <shobjidl.h>
#include <shlguid.h>
#include <shlwapi.h>
#include <stgprop.h>
#include <strsafe.h>

#include "enumpidl.h"
#include "simplepidl.h"

// {1A67366A-3641-4575-BCFF-8D574C6F68F0}
DEFINE_GUID(CLSID_ExampleShellFolder2,
0x1A67366A, 0x3641, 0x4575, 0xBC, 0xFF, 0x8D, 0x57, 0x4C, 0x6F, 0x68, 0xF0);

class CExampleShellFolder2 : public IShellFolder2, public IPersistFolder2  // No IShellDetails, because systems that need it do not have system-provided IShellView
{
private:
    ULONG m_ulLocks;

    PIDLIST_ABSOLUTE m_lpidl;

private:
    ~CExampleShellFolder2();

protected:
    STDMETHODIMP P_GetColumnCaption(PCUITEMID_CHILD lpidl, const SHCOLUMNID* lpScid, VARIANT* lpVarOut, LPWSTR lpszBuffer, UINT uBufferSize);

public:
    CExampleShellFolder2();

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

STDMETHODIMP CExampleShellFolder2::P_GetColumnCaption(PCUITEMID_CHILD lpidl, const SHCOLUMNID* lpScid, VARIANT* lpVarOut, LPWSTR lpszBuffer, UINT uBufferSize)
{
    HRESULT hr;

    if(IsEqualGUID(lpScid->fmtid, FMTID_Storage))
    {
        switch(lpScid->pid)
        {
            case PID_STG_NAME:
                // TODO: Obtain name from pidl
                if(lpVarOut)
                {
                    lpVarOut->vt = VT_BSTR;
                    lpVarOut->bstrVal = SysAllocString(L"Item Name (Variant)");

                    hr = lpVarOut->bstrVal ? S_OK : E_OUTOFMEMORY;
                }
                else
                {
                    hr = StringCchCopyW(lpszBuffer, uBufferSize, L"Item Name (String)");
                }

                return hr;
        }
    }

    if(lpVarOut)
    {
        VariantInit(lpVarOut);
    }
    else
    {
        lpszBuffer[0] = 0;
    }

    return S_OK;
}

CExampleShellFolder2::CExampleShellFolder2()
    : m_ulLocks(1UL)
    , m_lpidl(NULL)
{
}

CExampleShellFolder2::~CExampleShellFolder2()
{
    if(m_lpidl)
    {
        ILFree(m_lpidl);
        m_lpidl = NULL;
    }
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
    SHITEMID Shid[2] = { sizeof(SHITEMID) };
    LPITEMIDLIST lpList[1];

    lpList[0] = (LPITEMIDLIST)Shid;

    if(_ARRAYSIZE(lpList))
    {
        return CreateEnumIDListFromArray(lpList, _ARRAYSIZE(lpList), IID_IEnumIDList, (LPVOID*)lppEnumIDListOut);
    }

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
    HRESULT hr;

    if(IsEqualIID(riid, IID_IShellView))
    {
        IShellView* lpShellView = NULL;
        SFV_CREATE Sfv = { sizeof(Sfv) };

        Sfv.pshf = static_cast< IShellFolder* >(this);

        hr = SHCreateShellFolderView(&Sfv, &lpShellView);

        if(!FAILED(hr))
        {
            hr = lpShellView->QueryInterface(riid, lppOut);

            lpShellView->Release();
        }
    }
    else
    {
        hr = E_NOINTERFACE;
    }

    return hr;
}

STDMETHODIMP CExampleShellFolder2::GetAttributesOf(UINT uCount, PCUITEMID_CHILD_ARRAY lppidl, SFGAOF* lpulInOut)
{
    if(uCount==1U)
    {
        lpulInOut[0]&= 0;  // retrieved attributes

        return S_OK;
    }

    return E_INVALIDARG;
}

STDMETHODIMP CExampleShellFolder2::GetUIObjectOf(HWND hWndOwner, UINT uCount, PCUITEMID_CHILD_ARRAY lppidl, REFIID riid, UINT* lpuInOut, LPVOID* lppOut)
{
    HRESULT hr;

    if(IsEqualIID(riid, IID_IExtractIconA) || IsEqualIID(riid, IID_IExtractIconW))
    {
        wchar_t szDummyPath[MAX_PATH];

        hr = StringCchPrintfW(szDummyPath, _ARRAYSIZE(szDummyPath), L"C:\\~%ls", L"Display name.txt");

        if(!FAILED(hr))
        {
            WIN32_FIND_DATAW Wfd = { 0 };
            PIDLIST_ABSOLUTE lpidl = NULL;

            Wfd.dwFileAttributes = FILE_ATTRIBUTE_NORMAL;

            hr = CreateSimpleIDListFromPath(hWndOwner, szDummyPath, &Wfd, &lpidl);

            if(!FAILED(hr))
            {
                IShellFolder* lpFolder = NULL;
                PCUITEMID_CHILD lpidlChild = NULL;

                hr = SHBindToParent(lpidl, IID_PPV_ARGS(&lpFolder), &lpidlChild);

                if(!FAILED(hr))
                {
                    hr = lpFolder->GetUIObjectOf(hWndOwner, 1, &lpidlChild, riid, NULL, lppOut);

                    lpFolder->Release();
                }

                ILFree(lpidl);
            }
        }
    }
    else
    {
        hr = E_NOINTERFACE;
    }

    return hr;
}

STDMETHODIMP CExampleShellFolder2::GetDisplayNameOf(PCUITEMID_CHILD lpidl, SHGDNF dwFlags, LPSTRRET lpsrName)
{
    HRESULT hr;

    if(lpidl && lpsrName)
    {
        hr = SHStrDupW(L"Display Name", &lpsrName->pOleStr);

        if(!FAILED(hr))
        {
            lpsrName->uType = STRRET_WSTR;
        }
    }
    else
    {
        hr = E_INVALIDARG;
    }

    return hr;
}

STDMETHODIMP CExampleShellFolder2::SetNameOf(HWND hWndOwner, PCUITEMID_CHILD lpidl, LPCOLESTR lpszName, SHGDNF dwFlags, PITEMID_CHILD* lppidlOut)
{
    lppidlOut[0] = NULL;

    return E_NOTIMPL;
}

STDMETHODIMP CExampleShellFolder2::EnumSearches(IEnumExtraSearch** lppEnum)
{
    if(lppEnum)
    {
        lppEnum[0] = NULL;

        return E_NOINTERFACE;
    }

    return E_INVALIDARG;
}

STDMETHODIMP CExampleShellFolder2::GetDefaultColumn(DWORD dwReserved, ULONG* lpulSort, ULONG* lpulDisplay)
{
    if(lpulSort && lpulDisplay)
    {
        lpulSort[0] = 0UL;
        lpulDisplay[0] = 0UL;

        return S_OK;
    }

    return E_INVALIDARG;
}

STDMETHODIMP CExampleShellFolder2::GetDefaultColumnState(UINT uColumn, SHCOLSTATEF* lpcsFlags)
{
    if(lpcsFlags)
    {
        if(uColumn<1U)
        {
            lpcsFlags[0] = SHCOLSTATE_ONBYDEFAULT|SHCOLSTATE_TYPE_STR;

            return S_OK;
        }

        return E_FAIL;
    }

    return E_INVALIDARG;
}

STDMETHODIMP CExampleShellFolder2::GetDefaultSearchGUID(GUID* lpGuid)
{
    return E_NOTIMPL;
}

STDMETHODIMP CExampleShellFolder2::GetDetailsEx(PCUITEMID_CHILD lpidl, const SHCOLUMNID* lpScid, VARIANT* lpVarOut)
{
    HRESULT hr;

    if(lpidl && lpScid && lpVarOut)
    {
        hr = P_GetColumnCaption(lpidl, lpScid, lpVarOut, NULL, 0);
    }
    else
    {
        hr = E_INVALIDARG;
    }

    return hr;
}

STDMETHODIMP CExampleShellFolder2::GetDetailsOf(PCUITEMID_CHILD lpidl, UINT uColumn, SHELLDETAILS* lpSd)
{
    HRESULT hr;

    if(lpSd)
    {
        wchar_t szBuffer[256];

        lpSd->cxChar = 24;

        if(lpidl==NULL)
        {// NULL item means information about the columns themselves
            switch(uColumn)
            {
                case 0:
                    lpSd->fmt = LVCFMT_LEFT;

                    hr = StringCchCopyW(szBuffer, _ARRAYSIZE(szBuffer), L"Name");
                    break;
                default:
                    // end of column enumeration
                    hr = E_FAIL;
                    break;
            }
        }
        else
        {
            SHCOLUMNID Scid = { 0 };

            hr = MapColumnToSCID(uColumn, &Scid);

            if(!FAILED(hr))
            {
                hr = P_GetColumnCaption(lpidl, &Scid, NULL, szBuffer, _ARRAYSIZE(szBuffer));
            }
        }

        if(!FAILED(hr))
        {
            hr = SHStrDupW(szBuffer, &lpSd->str.pOleStr);

            if(!FAILED(hr))
            {
                lpSd->str.uType = STRRET_WSTR;
            }
        }
    }
    else
    {
        hr = E_INVALIDARG;
    }

    return hr;
}

STDMETHODIMP CExampleShellFolder2::MapColumnToSCID(UINT uColumn, SHCOLUMNID* lpScid)
{
    if(lpScid)
    {
        switch(uColumn)
        {
            case 0:
                lpScid->fmtid = FMTID_Storage;
                lpScid->pid   = PID_STG_NAME;
            default:
                return E_FAIL;
        }

        return S_OK;
    }

    return E_INVALIDARG;
}

STDMETHODIMP CExampleShellFolder2::GetClassID(CLSID* pclsid)
{
    pclsid[0] = CLSID_ExampleShellFolder2;

    return S_OK;
}

STDMETHODIMP CExampleShellFolder2::Initialize(PCIDLIST_ABSOLUTE lpidl)
{
    HRESULT hr;

    if(lpidl)
    {
        if(m_lpidl)
        {
            ILFree(m_lpidl);
            m_lpidl = NULL;
        }

        m_lpidl = ILCloneFull(lpidl);

        if(m_lpidl)
        {
            hr = S_OK;
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }
    else
    {
        hr = E_INVALIDARG;
    }

    return hr;
}

STDMETHODIMP CExampleShellFolder2::GetCurFolder(PIDLIST_ABSOLUTE* lppidl)
{
    HRESULT hr;

    if(lppidl)
    {
        PIDLIST_ABSOLUTE lpidl = NULL;

        if(m_lpidl)
        {
            lpidl = ILCloneFull(m_lpidl);

            if(lpidl)
            {
                hr = S_OK;
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }
        else
        {
            hr = S_FALSE;
        }

        lppidl[0] = lpidl;
    }
    else
    {
        hr = E_INVALIDARG;
    }

    return hr;
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

        ExampleShellFolder->Release();
        ExampleShellFolder = NULL;

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
