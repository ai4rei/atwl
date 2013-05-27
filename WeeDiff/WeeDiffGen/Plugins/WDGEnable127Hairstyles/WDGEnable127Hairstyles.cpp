#include <stdlib.h>
#include <crtdbg.h>

#ifdef _DEBUG
	#ifndef DBG_NEW
		#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
		#define new DBG_NEW
	#endif // DEBUG_NEW
#endif  // _DEBUG

#include "WDGEnable127Hairstyles.h"

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
		TEXT("Enable 127 Hairstyles"),
		TEXT("Allows you to use more than the default max. 27 hairstyles."),
		TEXT("[Add]"),
		TEXT(""),
		TEXT("Shinryo"),
		1,
		0,
		{ 0xdbda071c, 0x4218, 0x4803, { 0x9f, 0x9e, 0xd1, 0x56, 0xcb, 0x7a, 0xfa, 0x39 } }
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
		sFindData.lpData = "C0 CE B0 A3 C1 B7 5C B8 D3 B8 AE C5 EB 5C 25 73 5C 25 73";
		sFindData.chWildCard = '\xAB';
		sFindData.uMask = WFD_PATTERN | WFD_WILDCARD;

		m_dgc->Matches(CBAddOffset, &sFindData);

		m_offsets = NULL;

		if(offsets.size() < 2)
			throw "There have to be at least 2 matches!";

		uPart = 2;

		ZeroMemory(&sFindData, sizeof(sFindData));
		sFindData.lpData = "\x64";
		sFindData.uDataSize = 1;

		m_dgc->Replace(CBAddDiffData, offsets[0] + 18, &sFindData);
		m_dgc->Replace(CBAddDiffData, offsets[1] + 18, &sFindData);

		offsets.clear();

		uPart = 3;

		m_offsets = &offsets;

		ZeroMemory(&sFindData, sizeof(sFindData));
		sFindData.lpData = "7C 05 83 F8 1B 7E 06";
		sFindData.chWildCard = '\xAB';
		sFindData.uMask = WFD_PATTERN | WFD_WILDCARD;

		m_dgc->Matches(CBAddOffset, &sFindData);

		m_offsets = NULL;

		if(offsets.size() < 2)
			throw "There have to be at least 2 matches!";

		uPart = 4;

		ZeroMemory(&sFindData, sizeof(sFindData));
		sFindData.lpData = "\x7F";
		sFindData.uDataSize = 1;

		m_dgc->Replace(CBAddDiffData, offsets[0] + 4, &sFindData);
		m_dgc->Replace(CBAddDiffData, offsets[1] + 4, &sFindData);

		offsets.clear();

		uPart = 5;

		m_offsets = &offsets;

		ZeroMemory(&sFindData, sizeof(sFindData));
		sFindData.lpData = "8B 14 96 8B 89 AB AB 00 00";
		sFindData.chWildCard = '\xAB';
		sFindData.uMask = WFD_PATTERN | WFD_WILDCARD;

		m_dgc->Matches(CBAddOffset, &sFindData);

		m_offsets = NULL;

		if(offsets.size() < 2)
			throw "There have to be at least 2 matches!";

		uPart = 6;

		ZeroMemory(&sFindData, sizeof(sFindData));
		sFindData.lpData = "\x90\x90\x90";
		sFindData.uDataSize = 3;

		m_dgc->Replace(CBAddDiffData, offsets[0], &sFindData);
		m_dgc->Replace(CBAddDiffData, offsets[1], &sFindData);

		offsets.clear();

		uPart = 7;

		m_offsets = &offsets;

		ZeroMemory(&sFindData, sizeof(sFindData));
		sFindData.lpData = "8B 04 82 8B 89 AB AB 00 00";
		sFindData.chWildCard = '\xAB';
		sFindData.uMask = WFD_PATTERN | WFD_WILDCARD;

		m_dgc->Matches(CBAddOffset, &sFindData);

		m_offsets = NULL;

		if(offsets.size() < 2)
			throw "There have to be at least 2 matches!";

		uPart = 8;

		ZeroMemory(&sFindData, sizeof(sFindData));
		sFindData.lpData = "\x90\x90\x90";
		sFindData.uDataSize = 3;

		m_dgc->Replace(CBAddDiffData, offsets[0], &sFindData);
		m_dgc->Replace(CBAddDiffData, offsets[1], &sFindData);

		offsets.clear();

		uPart = 9;

		ZeroMemory(&sFindData, sizeof(sFindData));
		sFindData.lpData = "83 AB AB 7E AB C6 AB 00";
		sFindData.chWildCard = '\xAB';
		sFindData.uMask = WFD_PATTERN | WFD_WILDCARD;

		uOffset = m_dgc->Match(&sFindData);

		ZeroMemory(&sFindData, sizeof(sFindData));
		sFindData.lpData = "\x7F";
		sFindData.uDataSize = 1;

		m_dgc->Replace(CBAddDiffData, uOffset + 2, &sFindData);

	}
	catch (LPCSTR lpszMsg)
	{
		sprintf_s(szMsg, 256, "WDGEnable127Hairstyles :: Part %d :: %s", uPart, lpszMsg);
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