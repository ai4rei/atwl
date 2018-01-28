#define INITGUID

#include <urlmon.h>
#include <windows.h>

#include <btypes.h>
#include <bvbase64.h>
#include <urlcode.h>

// {9AFB53E0-EBAD-11E5-A77F-0011096B4C08}
DEFINE_GUID(CLSID_DataUri,
0x9afb53e0, 0xebad, 0x11e5, 0xa7, 0x7f, 0x0, 0x11, 0x9, 0x6b, 0x4c, 0x8);

class CDataUri : public IInternetProtocol
{
private:
	LPBYTE m_lpucData;
	LPWSTR m_lpszMimeType;
	ULONG m_ulDataRead;
	ULONG m_ulDataSize;
	ULONG m_ulLocks;
	bool m_bIsBase64;

protected:
	CDataUri();
	~CDataUri();

	STDMETHODIMP_(bool) P_ParseUrlProtocol(LPCWSTR lpszData, LPCWSTR* lppszNext);
	STDMETHODIMP_(bool) P_ParseUrlMediaType(LPCWSTR lpszData, LPCWSTR* lppszNext);
	STDMETHODIMP_(bool) P_ParseUrlExtension(LPCWSTR lpszData, LPCWSTR* lppszNext);
	STDMETHODIMP_(bool) P_ParseUrlData(LPCWSTR lpszData, LPCWSTR* lppszNext);
	STDMETHODIMP P_ParseUrl(LPCWSTR lpszUrl, LPCWSTR* lppszData);
	STDMETHODIMP P_ExtractData(LPCWSTR lpszData);

public:
	// IUnknown
	STDMETHODIMP QueryInterface(REFIID riid, LPVOID* lppOut);
	STDMETHODIMP_(ULONG) AddRef();
	STDMETHODIMP_(ULONG) Release();

	// IInternetProtocolRoot
	STDMETHODIMP Abort(HRESULT hrReason, DWORD dwOptions);
	STDMETHODIMP Continue(PROTOCOLDATA* lpProtocolData);
	STDMETHODIMP Resume();
	STDMETHODIMP Start(LPCWSTR szUrl, IInternetProtocolSink* lpOIProtSink, IInternetBindInfo* lpOIBindInfo, DWORD grfPI, HANDLE_PTR dwReserved);
	STDMETHODIMP Suspend();
	STDMETHODIMP Terminate(DWORD dwOptions);

	// IInternetProtocol
	STDMETHODIMP LockRequest(DWORD dwOptions);
	STDMETHODIMP Read(LPVOID lpBuffer, ULONG ulBufferSize, ULONG* lpulRead);
	STDMETHODIMP Seek(LARGE_INTEGER ullMove, DWORD dwOrigin, ULARGE_INTEGER* lpullNewPosition);
	STDMETHODIMP UnlockRequest();

	friend class CDataUriClassFactory;
};

class CDataUriClassFactory : public IClassFactory
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

static CDataUriClassFactory l_ClassFactory;  // keep a single global instance
static ULONG l_ulLocks = 0UL;

// CDataUri
//

CDataUri::CDataUri()
	: m_lpucData(NULL)
	, m_lpszMimeType(NULL)
	, m_ulDataRead(0UL)
	, m_ulDataSize(0UL)
	, m_ulLocks(1UL)
	, m_bIsBase64(false)
{
}

CDataUri::~CDataUri()
{
	if(m_lpucData)
	{
		delete[] m_lpucData;
		m_lpucData = NULL;
	}

	if(m_lpszMimeType)
	{
		delete[] m_lpszMimeType;
		m_lpszMimeType = NULL;
	}
}

STDMETHODIMP_(bool) CDataUri::P_ParseUrlProtocol(LPCWSTR lpszData, LPCWSTR* lppszNext)
{
	if(
		lpszData[0]=='d'
		&&
		lpszData[1]=='a'
		&&
		lpszData[2]=='t'
		&&
		lpszData[3]=='a'
		&&
		lpszData[4]==':'
	)
	{
		lppszNext[0] = &lpszData[5];

		return true;
	}

	return false;
}

STDMETHODIMP_(bool) CDataUri::P_ParseUrlMediaType(LPCWSTR lpszData, LPCWSTR* lppszNext)
{
	LPCWSTR lpszBegin = lpszData;

	if(lpszData[0]!=';' && lpszData[0]!=',')
	{
		for(; (lpszData[0]>='a' && lpszData[0]<='z'); lpszData++);

		if(lpszData[0]!='/')
		{
			return false;
		}
		lpszData++;

		for(; (lpszData[0]>='a' && lpszData[0]<='z') || (lpszData[0]>='0' && lpszData[0]<='9') || lpszData[0]=='-' || lpszData[0]=='.' || lpszData[0]=='+'; lpszData++);
	}

	while(lpszData[0]==';')
	{
		LPCWSTR lpszBackup = lpszData;

		lpszData++;

		for(; (lpszData[0]>='a' && lpszData[0]<='z') || (lpszData[0]>='A' && lpszData[0]<='Z') || (lpszData[0]>='0' && lpszData[0]<='9') || lpszData[0]=='-'; lpszData++);

		if(lpszData[0]==',')
		{// this was an extension
			lpszData = lpszBackup;
			break;
		}
		else
		if(lpszData[0]!='=')
		{
			return false;
		}

		for(; lpszData[0] && lpszData[0]!=';' && lpszData[0]!=','; lpszData++);
	}

	if(lpszData[0]==';' || lpszData[0]==',')
	{
		if((lpszData-lpszBegin)>0)
		{
			m_lpszMimeType = new WCHAR[lpszData-lpszBegin+1];
			wcsncpy(m_lpszMimeType, lpszBegin, lpszData-lpszBegin);
			m_lpszMimeType[lpszData-lpszBegin] = 0;
		}

		lppszNext[0] = lpszData;

		return true;
	}

	return false;
}

STDMETHODIMP_(bool) CDataUri::P_ParseUrlExtension(LPCWSTR lpszData, LPCWSTR* lppszNext)
{
	if(lpszData[0]==';')
	{
		lpszData++;

		if(
			lpszData[0]=='b'
			&&
			lpszData[1]=='a'
			&&
			lpszData[2]=='s'
			&&
			lpszData[3]=='e'
			&&
			lpszData[4]=='6'
			&&
			lpszData[5]=='4'
		)
		{
			m_bIsBase64 = true;

			lppszNext[0] = &lpszData[6];

			return true;
		}

		return false;
	}

	return true;
}

STDMETHODIMP_(bool) CDataUri::P_ParseUrlData(LPCWSTR lpszData, LPCWSTR* lppszNext)
{
	if(lpszData[0]==',')
	{
		lppszNext[0] = &lpszData[1];

		return true;
	}

	return false;
}

STDMETHODIMP CDataUri::P_ParseUrl(LPCWSTR lpszUrl, LPCWSTR* lppszData)
{// /^data:([a-z]+\\/[a-z\\-]+)?(;[a-z\\-]+=[0-9a-z\\-])*(;base64)?,(.*)$/i
	HRESULT hr = S_FALSE;
	LPCWSTR lpszData = lpszUrl;

	for(;;)
	{
		if(!P_ParseUrlProtocol(lpszData, &lpszData))
		{
			break;
		}

		if(!P_ParseUrlMediaType(lpszData, &lpszData))
		{
			break;
		}

		if(!P_ParseUrlExtension(lpszData, &lpszData))
		{
			break;
		}

		if(!P_ParseUrlData(lpszData, &lpszData))
		{
			break;
		}

		lppszData[0] = lpszData;
		hr = S_OK;
		break;
	}

	return hr;
}

STDMETHODIMP CDataUri::P_ExtractData(LPCWSTR lpszData)
{
	size_t uLength;
	HRESULT hr = S_FALSE;

	if(UrlDecodeW(lpszData, NULL, 0, &uLength))
	{
		ubyte_t* lpucData = new ubyte_t[uLength];

		if(UrlDecodeW(lpszData, lpucData, uLength, &uLength))
		{
			if(m_bIsBase64)
			{
				size_t uDecodeLength = BvBase64DecodedLength(uLength);
				ubyte_t* lpucDecode = new ubyte_t[uDecodeLength];

				if(BvBase64DecodeA((const char*)lpucData, uLength, lpucDecode, uDecodeLength, &uDecodeLength))
				{
					m_lpucData = lpucDecode;
					m_ulDataSize = uDecodeLength;

					hr = S_OK;
				}
				else
				{
					delete[] lpucDecode;
					lpucDecode = NULL;
				}

				delete[] lpucData;
				lpucData = NULL;
			}
			else
			{
				m_lpucData = lpucData;
				m_ulDataSize = uLength;

				hr = S_OK;
			}
		}
		else
		{
			delete[] lpucData;
			lpucData = NULL;
		}
	}

	return hr;
}

STDMETHODIMP CDataUri::QueryInterface(REFIID riid, LPVOID* lppOut)
{
	if(lppOut)
	{
		if(IsEqualIID(riid, IID_IUnknown))
		{
			lppOut[0] = static_cast< IUnknown* >(this);
		}
		else
		if(IsEqualIID(riid, IID_IInternetProtocolRoot))
		{
			lppOut[0] = static_cast< IInternetProtocolRoot* >(this);
		}
		else
		if(IsEqualIID(riid, IID_IInternetProtocol))
		{
			lppOut[0] = static_cast< IInternetProtocol* >(this);
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

STDMETHODIMP_(ULONG) CDataUri::AddRef()
{
	return InterlockedIncrement((LPLONG)&m_ulLocks);
}

STDMETHODIMP_(ULONG) CDataUri::Release()
{
	ULONG ulResult = InterlockedIncrement((LPLONG)&m_ulLocks);

	if(!ulResult)
	{
		delete this;
	}

	return ulResult;
}

STDMETHODIMP CDataUri::Abort(HRESULT hrReason, DWORD dwOptions)
{
	return S_OK;
}

STDMETHODIMP CDataUri::Continue(PROTOCOLDATA* lpProtocolData)
{
	return S_OK;
}

STDMETHODIMP CDataUri::Resume()
{
	return S_OK;
}

STDMETHODIMP CDataUri::Start(LPCWSTR szUrl, IInternetProtocolSink* lpOIProtSink, IInternetBindInfo* lpOIBindInfo, DWORD grfPI, HANDLE_PTR dwReserved)
{
	HRESULT hr;

	for(;;)
	{
		LPCWSTR lpszData = NULL;

		hr = P_ParseUrl(szUrl, &lpszData);

		if(FAILED(hr) || hr==S_FALSE)
		{
			break;
		}

		hr = P_ExtractData(lpszData);

		if(FAILED(hr))
		{
			break;
		}

		hr = lpOIProtSink->ReportData(BSCF_LASTDATANOTIFICATION, m_ulDataSize, m_ulDataSize);

		if(FAILED(hr))
		{
			break;
		}

		hr = lpOIProtSink->ReportProgress(BINDSTATUS_MIMETYPEAVAILABLE, m_lpszMimeType==NULL ? L"text/plain;charset=US-ASCII" : m_lpszMimeType);

		if(FAILED(hr))
		{
			break;
		}

		break;
	}

	lpOIProtSink->ReportResult(hr, 0, NULL);

	// NOTE: Must not Release lpOIProtSink or lpOIBindInfo because
	// we do not own them.

	return hr;
}

STDMETHODIMP CDataUri::Suspend()
{
	return S_OK;
}

STDMETHODIMP CDataUri::Terminate(DWORD dwOptions)
{
	return S_OK;
}

STDMETHODIMP CDataUri::LockRequest(DWORD dwOptions)
{
	// no need for locking
	return S_OK;
}

STDMETHODIMP CDataUri::Read(LPVOID lpBuffer, ULONG ulBufferSize, ULONG* lpulRead)
{
	ulBufferSize = T_MIN(ulBufferSize, m_ulDataSize-m_ulDataRead);

	CopyMemory(lpBuffer, &m_lpucData[m_ulDataRead], ulBufferSize);

	m_ulDataRead+= ulBufferSize;
	lpulRead[0]  = ulBufferSize;

	return (m_ulDataRead==m_ulDataSize) ? S_FALSE : S_OK;
}

STDMETHODIMP CDataUri::Seek(LARGE_INTEGER ullMove, DWORD dwOrigin, ULARGE_INTEGER* lpullNewPosition)
{
	// not supported
	return E_FAIL;
}

STDMETHODIMP CDataUri::UnlockRequest()
{
	// not locked
	return S_OK;
}

// CDataUriClassFactory
//

STDMETHODIMP CDataUriClassFactory::QueryInterface(REFIID riid, LPVOID* lppOut)
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

STDMETHODIMP_(ULONG) CDataUriClassFactory::AddRef()
{
	return LockServer(TRUE);
}

STDMETHODIMP_(ULONG) CDataUriClassFactory::Release()
{
	return LockServer(FALSE);
}

STDMETHODIMP CDataUriClassFactory::CreateInstance(IUnknown* lpUnkOuter, REFIID riid, LPVOID* lppOut)
{
	if(lppOut)
	{
		if(lpUnkOuter)
		{
			return CLASS_E_NOAGGREGATION;
		}

		CDataUri* DataUri = NULL;

		try
		{
			DataUri = new CDataUri;
		}
		catch(...)
		{
			;
		}

		if(DataUri==NULL)
		{
			return E_OUTOFMEMORY;
		}

		HRESULT hr = DataUri->QueryInterface(riid, lppOut);

		DataUri->Release();

		return hr;
	}

	return E_INVALIDARG;
}

STDMETHODIMP CDataUriClassFactory::LockServer(BOOL fLock)
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
		if(IsEqualCLSID(rclsid, CLSID_DataUri))
		{
			return l_ClassFactory.QueryInterface(riid, lppOut);
		}

		lppOut[0] = NULL;

		return CLASS_E_CLASSNOTAVAILABLE;
	}

	return E_INVALIDARG;
}
