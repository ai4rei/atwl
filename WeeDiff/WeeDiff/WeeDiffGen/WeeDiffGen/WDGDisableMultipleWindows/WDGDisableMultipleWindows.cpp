#include <stdlib.h>
#include <crtdbg.h>

#ifdef _DEBUG
	#ifndef DBG_NEW
		#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
		#define new DBG_NEW
	#endif // DEBUG_NEW
#endif  // _DEBUG

#include "WDGDisableMultipleWindows.h"

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
		TEXT("Disable Multiple Windows"),
		TEXT("Prevents the client from creating more than one instance on all lang types."),
		TEXT("[Fix]"),
		TEXT(""),
		TEXT("Shinryo"),
		1,
		0,
		{ 0xe852542b, 0xc20b, 0x47a7, { 0xa1, 0x9c, 0x7a, 0x6a, 0xf4, 0x63, 0xf2, 0x6e } }
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
		try
		{
			ZeroMemory(&sFindData, sizeof(sFindData));
			sFindData.lpData = "E8 AB AB AB FF AB FF 15 AB AB AB 00 A1 AB AB AB 00";
			sFindData.lpszSection = ".text";
			sFindData.chWildCard = '\xAB';
			sFindData.uMask = WFD_PATTERN | WFD_SECTION | WFD_WILDCARD;

			uOffset = m_dgc->Match(&sFindData);
			uOffset += 12;
		}
		catch(LPCSTR)
		{
			ZeroMemory(&sFindData, sizeof(sFindData));
			sFindData.lpData = "E8 AB AB AB FF 6A 00 FF 15 AB AB AB 00 A1 AB AB AB 00";
			sFindData.lpszSection = ".text";
			sFindData.chWildCard = '\xAB';
			sFindData.uMask = WFD_PATTERN | WFD_SECTION | WFD_WILDCARD;

			uOffset = m_dgc->Match(&sFindData);
			uOffset += 13;
		}

		uPart = 2;

		ZeroMemory(&sFindData, sizeof(sFindData));
		sFindData.lpData = "\xB8\xFF\xFF\xFF";
		sFindData.uDataSize = 4;

		m_dgc->Replace(CBAddDiffData, uOffset, &sFindData);
	}
	catch (LPCSTR lpszMsg)
	{
		sprintf_s(szMsg, 256, "WDGDisableMultipleWindows :: Part %d :: %s", uPart, lpszMsg);
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