#include <stdlib.h>
#include <crtdbg.h>

#ifdef _DEBUG
	#ifndef DBG_NEW
		#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
		#define new DBG_NEW
	#endif // DEBUG_NEW
#endif  // _DEBUG

#include "WDGEnableAuraOverLvl99.h"

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
		TEXT("Enable Aura Over Level 99 And Level 150"),
		TEXT("Allows the client to display standard auras over level 99 and 3rd class auras over level 150."),
		TEXT("[UI]"),
		TEXT(""),
		TEXT("Shinryo"),
		1,
		0,
		{ 0xd243dd47, 0x9bcb, 0x443c, { 0x89, 0xd9, 0xe7, 0xb, 0x6, 0x4c, 0xb5, 0x32 } }
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
		ZeroMemory(&sFindData, sizeof(sFindData));
		sFindData.lpData = "83 3D AB AB AB 00 63 EB 0A 81 3D AB AB AB 00 96 00 00 00 75 13";
		sFindData.lpszSection = ".text";
		sFindData.chWildCard = '\xAB';
		sFindData.uMask = WFD_PATTERN | WFD_SECTION | WFD_WILDCARD;

		try
		{
			uOffset = m_dgc->Match(&sFindData);
		}
		catch (LPCSTR)
		{
			ZeroMemory(&sFindData, sizeof(sFindData));
			sFindData.lpData = "81 3D AB AB AB 00 96 00 00 00 EB AB 83 3D AB AB AB 00 63 75";
			sFindData.lpszSection = ".text";
			sFindData.chWildCard = '\xAB';
			sFindData.uMask = WFD_PATTERN | WFD_SECTION | WFD_WILDCARD;

			uOffset = m_dgc->Match(&sFindData);
		}

		uPart = 2;

		ZeroMemory(&sFindData, sizeof(sFindData));
		sFindData.lpData = "\x72";
		sFindData.uDataSize = 1;

		m_dgc->Replace(CBAddDiffData, uOffset + 19, &sFindData);
	}
	catch (LPCSTR lpszMsg)
	{
		sprintf_s(szMsg, 256, "WDGEnableAuraOverLvl99AndLvl150 :: Part %d :: %s", uPart, lpszMsg);
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