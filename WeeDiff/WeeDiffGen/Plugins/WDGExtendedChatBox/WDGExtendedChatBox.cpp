#include <stdlib.h>
#include <crtdbg.h>

#ifdef _DEBUG
	#ifndef DBG_NEW
		#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
		#define new DBG_NEW
	#endif // DEBUG_NEW
#endif  // _DEBUG

#include "WDGExtendedChatBox.h"

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
		TEXT("Extended Chat Box"),
		TEXT("Increases max input chars of main/battle chat box from 70 to 234."),
		TEXT("[UI]"),
		TEXT(""),
		TEXT("Shinryo"),
		1,
		0,
		{ 0x742a4bef, 0x963c, 0x412a, { 0x8c, 0x53, 0xfb, 0xd9, 0x8f, 0x37, 0x3a, 0xa3 } }
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
		sFindData.lpData = "C7 40 AB 46 00 00 00";  // TODO: 'C780 80000000 46000000' in newer clients

		sFindData.chWildCard = '\xAB';
		sFindData.lpszSection = ".text";
		sFindData.uMask = WFD_PATTERN | WFD_SECTION | WFD_WILDCARD;

		m_dgc->Matches(CBAddOffset, &sFindData);

		m_offsets = NULL;

		if(offsets.size() != 4)
			throw "There have to be exactly 4 matches!";

		uPart = 2;

		ZeroMemory(&sFindData, sizeof(sFindData));
		sFindData.lpData = "\xEA";
		sFindData.uDataSize = 1;

		m_dgc->Replace(CBAddDiffData, offsets[3] + 3, &sFindData);
	}
	catch (LPCSTR lpszMsg)
	{
		sprintf_s(szMsg, 256, "WDGExtendedChatBox :: Part %d :: %s", uPart, lpszMsg);
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