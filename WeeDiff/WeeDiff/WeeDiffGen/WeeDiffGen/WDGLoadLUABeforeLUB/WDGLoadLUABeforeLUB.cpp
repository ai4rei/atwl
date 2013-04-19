#include <stdlib.h>
#include <crtdbg.h>

#ifdef _DEBUG
	#ifndef DBG_NEW
		#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
		#define new DBG_NEW
	#endif // DEBUG_NEW
#endif  // _DEBUG

#include "WDGLoadLUABeforeLUB.h"

WDGPlugin* g_SelfReference = NULL;

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
		TEXT("Load LUA Before LUB"),
		TEXT("Allows you to load LUA files before LUB files are being loaded."),
		TEXT("[Data]"),
		TEXT(""),
		TEXT("Shinryo"),
		1,
		0,
		{ 0x6c189fc5, 0x8bb9, 0x4af3, { 0x85, 0x71, 0x6a, 0xd3, 0x2d, 0xfc, 0x7c, 0xd8 } },
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

DiffData* WDGPlugin::GeneratePatch()
{
	WeeDiffGenPlugin::FINDDATA sFindData = {0};
	CHAR szMsg[256];
	m_diffdata.clear();

	UINT32 uOffset = 0;

	try
	{
		ZeroMemory(&sFindData, sizeof(sFindData));
		sFindData.lpData = "'.lua' 00 00 00 00 '.lub'";
		sFindData.uMask = WFD_PATTERN;

		uOffset = m_dgc->Match(&sFindData);
	} 
	catch (LPCSTR lpszMsg)
	{
		sprintf_s(szMsg, _ARRAYSIZE(szMsg), "WDGLoadLUABeforeLUB :: Part 1 :: %s", lpszMsg);
		m_dgc->LogMsg(szMsg);
		return NULL;
	}

	try
	{
		ZeroMemory(&sFindData, sizeof(sFindData));
		sFindData.lpData = "'.lub' 00 00 00 00 '.lua'";
		sFindData.uMask = WFD_PATTERN;

		m_dgc->Replace(CBAddDiffData, uOffset, &sFindData);
	} 
	catch (LPCSTR lpszMsg)
	{
		sprintf_s(szMsg, _ARRAYSIZE(szMsg), "WDGLoadLUABeforeLUB :: Part 2 :: %s", lpszMsg);
		m_dgc->LogMsg(szMsg);
		return NULL;
	}

	return &m_diffdata;
}

DiffData* WDGPlugin::GetDiffData()
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