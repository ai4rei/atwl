#include <stdlib.h>
#include <crtdbg.h>

#ifdef _DEBUG
	#ifndef DBG_NEW
		#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
		#define new DBG_NEW
	#endif // DEBUG_NEW
#endif  // _DEBUG

#include "WDGIncreaseHeadgearViewID.h"

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
		TEXT("Increase Headgear View ID"),
		TEXT("Allows you to increase the maximal view id used in LUA script files."),
		TEXT("[Data]"),
		TEXT(""),
		TEXT("Shinryo"),
		1,
		0,
		{ 0xc9936036, 0xe041, 0x4c37, { 0xa9, 0x11, 0xd3, 0xd2, 0xd9, 0x38, 0x5b, 0x95 } }
	};

	return &wpi;
}

INT32 WDGPlugin::Enabled()
{
	ZeroMemory(m_szValue, MAX_VALUE * sizeof(TCHAR));
	m_dgc->DisplayInputBox(GetPluginInfo()->lpszDiffName, TEXT("How many IDs are required? (min: 1'000; max: 30'000)"), m_szValue, MAX_VALUE);
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
		m_dgc->LogMsg("WDGIncreaseHeadgearViewID :: Failed to convert wide character to multibyte!");
		return NULL;
	}
	else
	{
		WideCharToMultiByte(CP_ACP, 0, m_szValue, -1, szValue, MAX_VALUE, NULL, NULL);
		UINT32 uValue = atoi(szValue);

		if(uValue < 1000)
			uValue = 1000;
		else if(uValue > 30000)
			uValue = 30000;

		_stprintf_s(m_szValue, MAX_VALUE, TEXT("%d"), uValue);
		
		UINT32 uOffset = 0;
		UINT32 uPart = 1;

		try
		{
			ZeroMemory(&sFindData, sizeof(sFindData));
			sFindData.lpszSection = ".text";
			sFindData.chWildCard = '\xAB';
			sFindData.uMask = WFD_SECTION | WFD_WILDCARD;

			UINT32 uOldValue = 1000;

			UINT32 uClientDate = m_dgc->GetClientDate();

			if(uClientDate >= 20110126)
			{
				sFindData.lpData = new CHAR[8];
				memcpy(sFindData.lpData, "\x00\x68\x00\x00\x00\x00\x8D\x8E", 8);
				sFindData.uDataSize = 8;
			}
			else
			{
				sFindData.lpData = new CHAR[9];
				memcpy(sFindData.lpData, "\x00\x3D\x00\x00\x00\x00\x73\xAB\x8D", 9);
				sFindData.uDataSize = 9;
			}			
			memcpy(sFindData.lpData + 2, (CHAR *)&uOldValue, 4);

			uOffset = m_dgc->Match(&sFindData);

			delete[] sFindData.lpData;

			uPart = 2;

			ZeroMemory(&sFindData, sizeof(sFindData));
			sFindData.lpData = (CHAR *)&uValue;
			sFindData.uDataSize = 4;

			m_dgc->Replace(CBAddDiffData, uOffset + 2, &sFindData);

			uOffset += 9;

			uPart = 3;

			for(UINT32 i = 0; i < 2; i++)
			{
				ZeroMemory(&sFindData, sizeof(sFindData));
				sFindData.lpData = (CHAR *)&uOldValue;
				sFindData.uDataSize = 4;
				sFindData.uStart = uOffset;

				uOffset = m_dgc->Match(&sFindData);

				ZeroMemory(&sFindData, sizeof(sFindData));
				sFindData.lpData = (CHAR *)&uValue;
				sFindData.uDataSize = 4;

				m_dgc->Replace(CBAddDiffData, uOffset, &sFindData);

				uOffset += 4;
			}
		}
		catch (LPCSTR lpszMsg)
		{
			sprintf_s(szMsg, 256, "WDGIncreaseHeadgearViewID :: Part %d :: %s", uPart, lpszMsg);
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