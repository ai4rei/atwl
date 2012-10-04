#include <stdlib.h>
#include <crtdbg.h>

#ifdef _DEBUG
	#ifndef DBG_NEW
		#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
		#define new DBG_NEW
	#endif // DEBUG_NEW
#endif  // _DEBUG

#include "WDGUseRagnarokIcon.h"

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
		TEXT("Use Ragnarok Icon"),
		TEXT("Tells the client to use the default Ragnarok icon."),
		TEXT("[UI]"),
		TEXT(""),
		TEXT("Shinryo"),
		1,
		0,
		{ 0x81bac767, 0x6f2f, 0x49cf, { 0xae, 0xd2, 0xa9, 0x87, 0xfe, 0xa9, 0xf3, 0xc1 } }
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

	try
	{
		// MISSION: Find resource pointer to icongroups 114 and 119,
		// and replace the pointer of 114 with the one from 119.

		ZeroMemory(&sFindData, sizeof(sFindData));
		sFindData.lpData = "72 00 00 00 '??' 00 80 77 00 00 00 '??' 00 80";
		sFindData.lpszSection = ".rsrc";
		sFindData.chWildCard = '?';
		sFindData.uMask = WFD_PATTERN | WFD_WILDCARD | WFD_SECTION;

		uOffset = m_dgc->Match(&sFindData);
	}
	catch (LPCSTR lpszMsg)
	{
		sprintf_s(szMsg, 256, "WDGUseRagnarokIcon :: Part 1 :: %s", lpszMsg);
		m_dgc->LogMsg(szMsg);
		return NULL;
	}

	try
	{
		char szPatchPx[2] = { m_dgc->GetBYTE(uOffset+12), m_dgc->GetBYTE(uOffset+13) };

		ZeroMemory(&sFindData, sizeof(sFindData));
		sFindData.lpData = szPatchPx;
		sFindData.uDataSize = sizeof(szPatchPx);

		m_dgc->Replace(CBAddDiffData, uOffset+4, &sFindData);
	} 
	catch (LPCSTR lpszMsg)
	{
		sprintf_s(szMsg, 256, "WDGUseRagnarokIcon :: Part 2 :: %s", lpszMsg);
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