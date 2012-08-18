#include <stdlib.h>
#include <crtdbg.h>

#ifdef _DEBUG
	#ifndef DBG_NEW
		#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
		#define new DBG_NEW
	#endif // DEBUG_NEW
#endif  // _DEBUG

#include "WDGDisableSwearFilter.h"

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
		TEXT("Disable Swear Filter"),
		TEXT("Allows you to skip the swear prevention function."),
		TEXT("[UI]"),
		TEXT(""),
		TEXT("Shinryo"),
		1,
		0,
		{ 0xa7c0b1c3, 0xc331, 0x474a, { 0xae, 0xc3, 0x40, 0xb0, 0x70, 0x64, 0x4e, 0xfc } }
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
		std::vector<UINT32> offsets;
		m_offsets = &offsets;

		ZeroMemory(&sFindData, sizeof(sFindData));
		sFindData.lpData = "8B 44 24 04 50 E8 AB AB FF FF 33 C9 84 C0 0F 94 C1 8A C1 C2 04 00";
		sFindData.chWildCard = '\xAB';
		sFindData.uMask = WFD_PATTERN | WFD_WILDCARD;

		m_dgc->Matches(CBAddOffset, &sFindData);

		m_offsets = NULL;

		if(offsets.size() != 2)
			throw "There have to be exactly 2 matches!";

		uPart = 2;

		ZeroMemory(&sFindData, sizeof(sFindData));
		sFindData.lpData = "\x30\xC0";
		sFindData.uDataSize = 2;

		m_dgc->Replace(CBAddDiffData, offsets[0] + 17, &sFindData);
	}
	catch (LPCSTR lpszMsg)
	{
		sprintf_s(szMsg, 256, "WDGDisableSwearFilter :: Part %d :: %s", uPart, lpszMsg);
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