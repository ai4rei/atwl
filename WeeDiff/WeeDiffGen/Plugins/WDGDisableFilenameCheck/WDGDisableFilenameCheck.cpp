#include <stdlib.h>
#include <crtdbg.h>

#ifdef _DEBUG
	#ifndef DBG_NEW
		#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
		#define new DBG_NEW
	#endif // DEBUG_NEW
#endif  // _DEBUG

#include "WDGDisableFilenameCheck.h"

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
		TEXT("Disable RagexeRE Filename Check"),
		TEXT("Allows you to rename the executable to whatever you want."),
		TEXT("[Fix]"),
		TEXT(""),
		TEXT("Shinryo"),
		1,
		0,
		{ 0x7ab0d152, 0xa66a, 0x4caf, { 0x8f, 0xe5, 0x85, 0x6f, 0x91, 0xc1, 0x75, 0xe2 } },
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
	UINT32 uReplacePos = 11;

	try
	{
		ZeroMemory(&sFindData, sizeof(sFindData));
		sFindData.lpData = "E8 AB AB AB FF 39 AB AB AB AB 00 75 AB E8 AB AB FF FF 84 C0";
		sFindData.chWildCard = '\xAB';
		sFindData.uMask = WFD_PATTERN | WFD_WILDCARD;

		uOffset = m_dgc->Match(&sFindData);
	} 
	catch (LPCSTR)
	{
		try
		{
			ZeroMemory(&sFindData, sizeof(sFindData));
			sFindData.lpData = "E8 AB AB AB FF AB AB 39 AB AB AB AB 00 75 AB E8 AB AB FF FF 84 C0";
			sFindData.chWildCard = '\xAB';
			sFindData.uMask = WFD_PATTERN | WFD_WILDCARD;

			uOffset = m_dgc->Match(&sFindData);

			uReplacePos += 2;
		}
		catch (LPCSTR lpszMsg)
		{
			sprintf_s(szMsg, 256, "WDGDisableFilenameCheck :: Part 1 :: %s", lpszMsg);
			m_dgc->LogMsg(szMsg);
			return NULL;
		}
	}

	try
	{
		ZeroMemory(&sFindData, sizeof(sFindData));
		sFindData.lpData = "\xEB";
		sFindData.uDataSize = 1;

		m_dgc->Replace(CBAddDiffData, uOffset + uReplacePos, &sFindData, false);
	}
	catch (LPCSTR lpszMsg)
	{
		sprintf_s(szMsg, 256, "WDGDisableFilenameCheck :: Part 2 :: %s", lpszMsg);
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