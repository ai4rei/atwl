#include <windows.h>
#include <shlobj.h>

#include "enumpidl.h"

class CEnumIDList : public IEnumIDList
{
    ULONG m_ulLocks;

    LPITEMIDLIST* m_alpidl;
    ULONG m_ulpidlCount;
    ULONG m_ulpidlIdx;

protected:
    CEnumIDList(LPITEMIDLIST* alpidl, ULONG ulpidlCount);
    ~CEnumIDList();

public:
    // IUnknown
    STDMETHODIMP QueryInterface(REFIID riid, LPVOID* lppOut);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

    // IEnumIDList
    STDMETHODIMP Next(ULONG ulRead, LPITEMIDLIST* alpidl, ULONG* lpulRead);
    STDMETHODIMP Skip(ULONG ulSkip);
    STDMETHODIMP Reset();
    STDMETHODIMP Clone(IEnumIDList** lppOut);

    // CEnumIDList
    static STDMETHODIMP Create(LPITEMIDLIST* alpidl, ULONG ulpidlCount, REFIID riid, LPVOID* lppOut);
};

CEnumIDList::CEnumIDList(LPITEMIDLIST* alpidl, ULONG ulpidlCount)
{
    m_ulLocks = 1U;

    m_alpidl      = alpidl;
    m_ulpidlCount = ulpidlCount;
    m_ulpidlIdx   = 0UL;
}
CEnumIDList::~CEnumIDList()
{
    ULONG ulIdx;

    for(ulIdx = 0UL; ulIdx<m_ulpidlCount; ulIdx++)
    {
        ILFree(m_alpidl[ulIdx]);
        m_alpidl[ulIdx] = NULL;
    }

    CoTaskMemFree(m_alpidl);
    m_alpidl = NULL;
    m_ulpidlCount = 0U;
}

STDMETHODIMP CEnumIDList::QueryInterface(REFIID riid, LPVOID* lppOut)
{
    if(lppOut)
    {
        if(IsEqualIID(riid, IID_IUnknown))
        {
            lppOut[0] = static_cast< IUnknown* >(this);
        }
        else
        if(IsEqualIID(riid, IID_IEnumIDList))
        {
            lppOut[0] = static_cast< IEnumIDList* >(this);
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

STDMETHODIMP_(ULONG) CEnumIDList::AddRef()
{
    return InterlockedIncrement((LPLONG)&m_ulLocks);
}

STDMETHODIMP_(ULONG) CEnumIDList::Release()
{
    ULONG ulResult = InterlockedIncrement((LPLONG)&m_ulLocks);

    if(!ulResult)
    {
        delete this;
    }

    return ulResult;
}

STDMETHODIMP CEnumIDList::Next(ULONG ulRead, LPITEMIDLIST* alpidl, ULONG* lpulRead)
{
    HRESULT hr = S_FALSE;  // assume nothing found
    ULONG ulIdx;

    for(ulIdx = 0UL; ulIdx<ulRead && m_ulpidlIdx<m_ulpidlCount; ulIdx++, m_ulpidlIdx++)
    {
        alpidl[ulIdx] = ILClone(m_alpidl[m_ulpidlIdx]);

        if(alpidl[ulIdx])
        {
            hr = S_OK;
        }
        else
        {
            hr = E_OUTOFMEMORY;
            break;
        }
    }

    if(FAILED(hr))
    {// rollback
        while(ulIdx)
        {
            ulIdx--;

            ILFree(alpidl[ulIdx]);
            alpidl[ulIdx] = NULL;
        }
    }

    if(lpulRead)
    {
        lpulRead[0] = ulIdx;
    }

    return hr;
}

STDMETHODIMP CEnumIDList::Skip(ULONG ulSkip)
{
    m_ulpidlIdx+= ulSkip;

    return S_OK;
}

STDMETHODIMP CEnumIDList::Reset()
{
    m_ulpidlIdx = 0UL;

    return S_OK;
}

STDMETHODIMP CEnumIDList::Clone(IEnumIDList** lppOut)
{
    return CEnumIDList::Create(m_alpidl, m_ulpidlCount, IID_IEnumIDList, (LPVOID*)lppOut);
}

STDMETHODIMP CEnumIDList::Create(LPITEMIDLIST* alpidl, ULONG ulpidlCount, REFIID riid, LPVOID* lppOut)
{
    HRESULT hr;
    LPITEMIDLIST* alpidlClone = static_cast< LPITEMIDLIST* >(CoTaskMemAlloc(sizeof(LPITEMIDLIST)*ulpidlCount));

    if(alpidlClone)
    {
        ULONG ulIdx;

        for(ulIdx = 0UL; ulIdx<ulpidlCount; ulIdx++)
        {
            alpidlClone[ulIdx] = ILClone(alpidl[ulIdx]);

            if(!alpidlClone[ulIdx])
            {
                break;
            }
        }

        if(ulIdx==ulpidlCount)
        {
            CEnumIDList* lpList = new CEnumIDList(alpidlClone, ulpidlCount);  // takes over

            hr = lpList->QueryInterface(riid, lppOut);

            lpList->Release();
        }
        else
        {// rollback
            while(ulIdx)
            {
                ulIdx--;

                ILFree(alpidlClone[ulIdx]);
                alpidlClone[ulIdx] = NULL;
            }

            CoTaskMemFree(alpidlClone);
            alpidlClone = NULL;

            hr = E_OUTOFMEMORY;
        }
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    return hr;
}

STDAPI CreateEnumIDListFromArray(LPITEMIDLIST* alpidl, ULONG ulpidlCount, REFIID riid, LPVOID* lppOut)
{
    return CEnumIDList::Create(alpidl, ulpidlCount, riid, lppOut);
}
