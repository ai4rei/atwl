#define INITGUID

#include <windows.h>

// {3834A400-E26F-11E6-A781-0011096B4C08}
DEFINE_GUID(CLSID_HelloWorld,
0x3834A400, 0xE26F, 0x11E6, 0xA7, 0x81, 0x00, 0x11, 0x09, 0x6B, 0x4C, 0x08);
DEFINE_GUID(IID_IHelloWorld,
0x3834A400, 0xE26F, 0x11E6, 0xA7, 0x81, 0x00, 0x11, 0x09, 0x6B, 0x4C, 0x08);

class IHelloWorld : public IDispatch
{
public:
    virtual STDMETHODIMP Hello() = 0;
};

class CHelloWorldClassFactory;

class CHelloWorld : public IHelloWorld
{
private:
    ULONG m_ulLocks;

private:
    ~CHelloWorld();

protected:
    CHelloWorld();

public:
    // IUnknown
    STDMETHODIMP QueryInterface(REFIID riid, LPVOID* lppOut);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

    // IDispatch
    STDMETHODIMP GetTypeInfoCount(UINT FAR* pctinfo);
    STDMETHODIMP GetTypeInfo(UINT itinfo, LCID lcid, ITypeInfo FAR* FAR* pptinfo);
    STDMETHODIMP GetIDsOfNames(REFIID riid, OLECHAR FAR* FAR* rgszNames, UINT cNames, LCID lcid, DISPID FAR* rgDispId);
    STDMETHODIMP Invoke(DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS FAR* pDispParams, VARIANT FAR* pVarResult, EXCEPINFO FAR* pExcepInfo, UINT FAR* puArgErr);

    // IHelloWorld
    STDMETHODIMP Hello();

    friend class CHelloWorldClassFactory;
};

class CHelloWorldClassFactory : public IClassFactory
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

static CHelloWorldClassFactory l_ClassFactory;
static ULONG l_ulLocks = 0UL;

// CHelloWorld
//

CHelloWorld::~CHelloWorld()
{
    ;
}

CHelloWorld::CHelloWorld()
{
    m_ulLocks = 0UL;
}

STDMETHODIMP CHelloWorld::QueryInterface(REFIID riid, LPVOID* lppOut)
{
    if(lppOut)
    {
        if(IsEqualIID(riid, IID_IUnknown))
        {
            lppOut[0] = static_cast< IUnknown* >(this);
        }
        else
        if(IsEqualIID(riid, IID_IHelloWorld))
        {
            lppOut[0] = static_cast< IHelloWorld* >(this);
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

STDMETHODIMP_(ULONG) CHelloWorld::AddRef()
{
    return InterlockedIncrement((LPLONG)&l_ulLocks);
}

STDMETHODIMP_(ULONG) CHelloWorld::Release()
{
    ULONG ulResult = InterlockedDecrement((LPLONG)&l_ulLocks);

    if(!ulResult)
    {
        delete this;
    }

    return ulResult;
}

STDMETHODIMP CHelloWorld::Hello()
{
    MessageBoxA(NULL, "Hello World!", "IHelloWorld", MB_OK);

    return S_OK;
}

// ClassFactory
//

STDMETHODIMP CHelloWorldClassFactory::QueryInterface(REFIID riid, LPVOID* lppOut)
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

STDMETHODIMP_(ULONG) CHelloWorldClassFactory::AddRef()
{
    return LockServer(TRUE);
}

STDMETHODIMP_(ULONG) CHelloWorldClassFactory::Release()
{
    return LockServer(FALSE);
}

STDMETHODIMP CHelloWorldClassFactory::CreateInstance(IUnknown* lpUnkOuter, REFIID riid, LPVOID* lppOut)
{
    if(lppOut)
    {
        lppOut[0] = NULL;

        if(lpUnkOuter)
        {
            return CLASS_E_NOAGGREGATION;
        }

        CHelloWorld* HelloWorld = NULL;

        try
        {
            HelloWorld = new CHelloWorld;
        }
        catch(...)
        {
            ;
        }

        if(HelloWorld==NULL)
        {
            return E_OUTOFMEMORY;
        }

        HRESULT hr = static_cast< IUnknown* >(HelloWorld)->QueryInterface(riid, lppOut);

        if(FAILED(hr))
        {
            delete HelloWorld;
        }

        return hr;
    }

    return E_INVALIDARG;
}

STDMETHODIMP CHelloWorldClassFactory::LockServer(BOOL fLock)
{
    (fLock ? InterlockedIncrement : InterlockedDecrement)((LPLONG)&l_ulLocks);

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
        if(IsEqualCLSID(rclsid, CLSID_HelloWorld))
        {
            return l_ClassFactory.QueryInterface(riid, lppOut);
        }

        lppOut[0] = NULL;

        return CLASS_E_CLASSNOTAVAILABLE;
    }

    return E_INVALIDARG;
}

BOOL CALLBACK DllMain(HINSTANCE hDll, DWORD dwReason, LPVOID lpReserved)
{
    switch(dwReason)
    {
        case DLL_PROCESS_ATTACH:
            DisableThreadLibraryCalls(hDll);
            break;
        case DLL_PROCESS_DETACH:
            break;
    }

    return TRUE;
}
