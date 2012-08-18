#include <stdlib.h>
#include <crtdbg.h>

#ifdef _DEBUG
	#ifndef DBG_NEW
		#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
		#define new DBG_NEW
	#endif // DEBUG_NEW
#endif  // _DEBUG

#include "WDGDisableNagleAlgorithm.h"
#include "InjectedCode.h"

WDGPlugin *g_SelfReference = NULL;

void WDGPlugin::Release()
{
	m_diffdata.clear();
	g_SelfReference = NULL;
	delete this;
}

void WDGPlugin::Free(LPVOID memory)
{
	delete memory;
	memory = NULL;
}

LPWDGPLUGININFO WDGPlugin::GetPluginInfo()
{
	static WDGPLUGININFO wpi = 
	{
		TEXT("Disable Nagle Algorithm"),
		TEXT("The Nagle Algorithm delays packet transfer by combining small packets into a large one reducing bandwidth while increasing latency."),
		TEXT("[Add]"),
		TEXT(""),
		TEXT("Shinryo"),
		1,
		0,
		{ 0x1e8b877c, 0xe364, 0x4aa4, { 0x9c, 0x41, 0xb6, 0xc4, 0x87, 0xcb, 0xd8, 0x72 } },
		TEXT("Recommended")
	};

	return &wpi;
}

INT32 WDGPlugin::Enabled()
{
	return 0;
}

INT32 WDGPlugin::Disabled()
{
	return 0;
}

LPCTSTR WDGPlugin::GetInputValue()
{
	return NULL;
}

DiffData *WDGPlugin::GeneratePatch()
{
	WeeDiffGenPlugin::FINDDATA sFindData = {0};
	CHAR szMsg[256];
	m_diffdata.clear();

	UINT32 uOffset = 0;
	UINT32 uPart = 1;

	try
	{	
		bool bCalledByDistance = false;
		UINT32 uCall = 0;
		UINT32 uTotalSize = 0;
		UINT32 uDeadBeefCounter = 0;

		const char strings[][20] =
		{
			"WS2_32.DLL",
			"setsockopt",
			"socket"
		};
		
		DWORD32 dwCodeAddr = (DWORD32)&mySocket;
		DWORD32 dwCodeAddrEnd = dwCodeAddr;
		DWORD32 dwDeadBeef = dwCodeAddrEnd;

		while(uDeadBeefCounter < 2)
		{
			dwDeadBeef = (DWORD)(*(DWORD *)(++dwCodeAddrEnd));

			if(dwDeadBeef == 0xDEADBEEF)
				uDeadBeefCounter++;
		}
			
		UINT32 uLoadLibrary = m_dgc->FindFunction("LoadLibraryA");
		UINT32 uGetProcAddress = m_dgc->FindFunction("GetProcAddress");
		UINT32 uGetModuleHandleA = m_dgc->FindFunction("GetModuleHandleA");

		uTotalSize = (dwCodeAddrEnd - dwCodeAddr) + 16 + strlen(strings[0]) + strlen(strings[1]) + strlen(strings[2]) + 3;
		UINT32 uDataPos = (dwCodeAddrEnd - dwCodeAddr) + 4;

		uPart = 2;

		UINT32 uFreeOffset = m_dgc->GetNextFreeOffset(uTotalSize);

		UCHAR *pCode = new UCHAR[uTotalSize];

		memcpy(pCode, (void *)dwCodeAddr, uDataPos);
		memcpy(pCode + uDataPos + 0, (CHAR*)&uLoadLibrary, 4);
		memcpy(pCode + uDataPos + 4, (CHAR*)&uGetProcAddress, 4);
		memcpy(pCode + uDataPos + 8, (CHAR*)&uGetModuleHandleA, 4);

		UINT32 uLastLen, uLastPos = 0;

		for(UINT32 i = 0; i < 3; i++)
		{
			uLastLen = strlen(strings[i]) + 1;		
			memcpy(pCode + uDataPos + 12 + uLastPos, strings[i], uLastLen);
			uLastPos += uLastLen;
		}

		ZeroMemory(&sFindData, sizeof(sFindData));
		sFindData.lpData = (CHAR *)pCode;
		sFindData.uDataSize = uTotalSize;
		
		m_dgc->Replace(CBAddDiffData, uFreeOffset, &sFindData);
		delete[] pCode;

		uPart = 3;

		try
		{
			ZeroMemory(&sFindData, sizeof(sFindData));
			sFindData.lpData = "E8 AB AB 00 00 6A 00 6A 01 6A 02 FF 15 AB AB AB 00";
			sFindData.chWildCard = '\xAB';
			sFindData.uMask = WFD_PATTERN | WFD_WILDCARD;
			uOffset = m_dgc->Match(&sFindData);

			uCall = m_dgc->GetDWORD32(uOffset + 11 + 2);
		}
		catch (LPCSTR)
		{
			ZeroMemory(&sFindData, sizeof(sFindData));
			sFindData.lpData = "E8 AB AB 00 00 6A 00 6A 01 6A 02 E8 AB AB AB 00";
			sFindData.chWildCard = '\xAB';
			sFindData.uMask = WFD_PATTERN | WFD_WILDCARD;
			uOffset = m_dgc->Match(&sFindData);

			uCall = m_dgc->GetDWORD32(uOffset + 11 + 1);
			uCall = m_dgc->Raw2Rva(uOffset + 11 + uCall + 5);
			bCalledByDistance = true;
		}

		uPart = 4;

		ZeroMemory(&sFindData, sizeof(sFindData));
		sFindData.lpData = new CHAR[6];
		sFindData.uDataSize = 6;
		
		memcpy(sFindData.lpData, "\xFF\x25", 2);
		memcpy(sFindData.lpData + 2, (CHAR *)&uCall, 4);

		try
		{
			uOffset = m_dgc->Match(&sFindData);
		}
		catch (LPCSTR)
		{
			/* just ignore it */
		}

		delete[] sFindData.lpData;

		uPart = 5;

		UINT32 uFreeOffsetRva = m_dgc->Raw2Rva(uFreeOffset);

		ZeroMemory(&sFindData, sizeof(sFindData));
		sFindData.lpData = (CHAR *)&uFreeOffsetRva;
		sFindData.uDataSize = 4;

		m_dgc->Replace(CBAddDiffData, uOffset + 2, &sFindData);

		if(bCalledByDistance == false)
		{
			std::vector<UINT32> offsets;
			m_offsets = &offsets;

			ZeroMemory(&sFindData, sizeof(sFindData));
			sFindData.lpData = new CHAR[6];
			sFindData.uDataSize = 6;

			memcpy(sFindData.lpData, "\xFF\x15", 2);
			memcpy(sFindData.lpData + 2, (CHAR *)&uCall, 4);

			m_dgc->Matches(CBAddOffset, &sFindData);

			delete[] sFindData.lpData;

			m_offsets = NULL;

			CHAR szData[6];
			szData[0] = '\xE8';
			szData[5] = '\x90';
			for(UINT32 i = 0; i < offsets.size(); i++)
			{
				UINT32 uValue = m_dgc->Raw2Rva(uFreeOffset) - m_dgc->Raw2Rva(offsets[i]) - 5;
				memcpy(szData + 1, (CHAR *)&uValue, 4);

				ZeroMemory(&sFindData, sizeof(sFindData));
				sFindData.lpData = szData;
				sFindData.uDataSize = 6;

				m_dgc->Replace(CBAddDiffData, offsets[i], &sFindData);
			}
		}
	}
	catch (LPCSTR lpszMsg)
	{
		sprintf_s(szMsg, 256, "WDGEnableMultipleGRF :: Part %d :: %s", uPart, lpszMsg);
		m_dgc->LogMsg(szMsg);
		return NULL;
	}

	return &m_diffdata;
}

DiffData *WDGPlugin::GetDiffData()
{
	if(m_diffdata.size() <= 0)
	{
		return NULL;
	}

	return &m_diffdata;
}

extern "C" __declspec(dllexport) WeeDiffGenPlugin::IWDGPlugin *InitPlugin(LPVOID lpData, USHORT unWeeDiffMajorVersion, USHORT unWeeDiffMinorVersion)
{
	// Enable functions to track down memory leaks.
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	if(g_SelfReference == NULL)
	{
		g_SelfReference = new WDGPlugin(lpData);
	}

	return g_SelfReference;
}

void WDGPlugin::CBAddDiffData(WeeDiffGenPlugin::LPDIFFDATA lpDiffData)
{
	if(g_SelfReference != NULL)
	{
		g_SelfReference->m_diffdata.push_back(*lpDiffData);
	}
}

void WDGPlugin::CBAddOffset(UINT32 uOffset)
{
	if(g_SelfReference != NULL && g_SelfReference->m_offsets != NULL)
	{
		g_SelfReference->m_offsets->push_back(uOffset);
	}
}