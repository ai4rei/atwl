#include <stdlib.h>
#include <crtdbg.h>

#ifdef _DEBUG
	#ifndef DBG_NEW
		#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
		#define new DBG_NEW
	#endif // DEBUG_NEW
#endif  // _DEBUG

#include "WDGAllowChatFlood.h"

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
		TEXT("Allow Chat Flood"),
		TEXT("Allows you to repeat the same line n-times."),
		TEXT("[UI]"),
		TEXT(""),
		TEXT("Shinryo"),
		1,
		0,
		{ 0xf3bde199, 0x4cf0, 0x4b50, { 0x92, 0x62, 0xef, 0x32, 0x8e, 0xe1, 0xb1, 0x2e } }
	};

	return &wpi;
}

INT32 WDGPlugin::Enabled()
{
	ZeroMemory(m_szValue, MAX_VALUE * sizeof(TCHAR));
	m_dgc->DisplayInputBox(GetPluginInfo()->lpszDiffName, TEXT("How many repeatable lines are allowed? (min: 3; max: 255)"), m_szValue, MAX_VALUE);
	m_dgc->UpdateListView();
	GeneratePatch();

	return 0;
}

INT32 WDGPlugin::Disabled()
{
	return 0;
}

LPCTSTR WDGPlugin::GetInputValue()
{
	return m_szValue;
}

DiffData *WDGPlugin::GeneratePatch()
{
	WeeDiffGenPlugin::FINDDATA sFindData = {0};
	CHAR szMsg[256];
	m_diffdata.clear();

	CHAR szValue[MAX_VALUE];
	ZeroMemory(szValue, MAX_VALUE);

	UINT32 uConvSize = WideCharToMultiByte(CP_ACP, 0, m_szValue, -1, NULL, 0, NULL, NULL);

	if(uConvSize > MAX_VALUE)
	{
		m_dgc->LogMsg("WDGAllowChatFlood :: Failed to convert wide character to multibyte!");
		return NULL;
	}
	else
	{
		WideCharToMultiByte(CP_ACP, 0, m_szValue, -1, szValue, MAX_VALUE, NULL, NULL);
		UINT32 uValue = atoi(szValue);

		if(uValue < 0x03)
			uValue = 0x3;
		else if(uValue > 0xFF)
			uValue = 0xFF;

		_stprintf_s(m_szValue, MAX_VALUE, TEXT("%d"), uValue);
		
		UINT32 uOffset = 0;
		UINT32 uPart = 1;

		try
		{
			ZeroMemory(&sFindData, sizeof(sFindData));
			sFindData.lpData = 
							 "833D ABABABAB 0A"  // CMP     DWORD PTR DS:[<g_serviceType>],0Ah
							 "74 AB"             // JE      ADDR v
							 "837C24 04 02"      // CMP     [ESP+4],2
							 "7C 47"             // JL      ADDR v
							 "6A 00"             // PUSH    0
							 ;
			sFindData.lpszSection = ".text";
			sFindData.chWildCard = '\xAB';
			sFindData.uMask = WFD_PATTERN | WFD_SECTION | WFD_WILDCARD;

			uOffset = m_dgc->Match(&sFindData);

			uPart = 2;

			ZeroMemory(&sFindData, sizeof(sFindData));
			sFindData.lpData = (CHAR *)&uValue;
			sFindData.uDataSize = 1;

			m_dgc->Replace(CBAddDiffData, uOffset + 13, &sFindData);
		}
		catch (LPCSTR lpszMsg)
		{
			sprintf_s(szMsg, 256, "WDGAllowChatFlood :: Part %d :: %s", uPart, lpszMsg);
			m_dgc->LogMsg(szMsg);
			return NULL;
		}
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