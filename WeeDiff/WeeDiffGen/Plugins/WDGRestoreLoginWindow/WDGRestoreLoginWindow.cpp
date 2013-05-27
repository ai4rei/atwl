#include <stdlib.h>
#include <crtdbg.h>

#ifdef _DEBUG
	#ifndef DBG_NEW
		#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
		#define new DBG_NEW
	#endif // DEBUG_NEW
#endif  // _DEBUG

#include "WDGRestoreLoginWindow.h"

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
		TEXT("Restore Login Window"),
		TEXT("Tells the client to use the old login interface prior to Gravitys new token based login system."),
		TEXT("[Fix]"),
		TEXT(""),
		TEXT("Shinryo"),
		1,
		0,
		{ 0x4cb932aa, 0x38de, 0x443f, { 0x8f, 0xb4, 0xb8, 0xd6, 0x92, 0x94, 0x5f, 0xf1 } },
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
	UINT32 uPart = 1;

	try
	{
		ZeroMemory(&sFindData, sizeof(sFindData));
		sFindData.lpData = "50 E8 AB AB AB FF 8B C8 E8 AB AB AB FF 50 B9 AB AB AB 00 E8 AB AB AB FF 80 3D AB AB AB 00 00 74 AB C6 AB AB AB AB 00 00 C7 AB AB 04 00 00 00 E9 AB AB 00 00";
		sFindData.lpszSection = ".text";
		sFindData.chWildCard = '\xAB';
		sFindData.uMask = WFD_PATTERN | WFD_SECTION | WFD_WILDCARD;

		uOffset = m_dgc->Match(&sFindData);

		uPart = 2;

		CHAR chMove[5];

		m_dgc->Read(uOffset + 14, (UCHAR *)chMove, 5);

		uPart = 3;

		ZeroMemory(&sFindData, sizeof(sFindData));
		sFindData.lpData = "'NUMACCOUNT'";
		sFindData.uMask = WFD_PATTERN;

		UINT32 uOffsetNumAccount = m_dgc->FindStr(&sFindData, true);

		uPart = 4;

		ZeroMemory(&sFindData, sizeof(sFindData));
		sFindData.lpData = new CHAR[34];
		sFindData.uDataSize = 34;
		sFindData.chWildCard = '\xAB';
		sFindData.lpszSection = ".text";
		sFindData.uMask = WFD_SECTION | WFD_WILDCARD;

		memcpy(sFindData.lpData, "\xB9\xAB\xAB\xAB\x00\xE8\xAB\xAB\xAB\xFF\x6A\x00\x6A\x00\x68\x00\x00\x00\x00\x8B\xF8\x8B\x17\x8B\x82\xAB\x00\x00\x00\x68\x23\x27\x00\x00", 34);
		memcpy(sFindData.lpData + 15, (CHAR*)&uOffsetNumAccount, 4);

		UINT32 uOffsetA = m_dgc->Match(&sFindData);

		delete[] sFindData.lpData;

		uPart = 5;

		UINT32 uCallA = m_dgc->GetDWORD32(uOffsetA + 6);
		UINT32 uCallB = ((uOffsetA + uCallA) - (uOffset + 26));

		ZeroMemory(&sFindData, sizeof(sFindData));
		sFindData.lpData = new CHAR[23];
		sFindData.uDataSize = 23;
		sFindData.chWildCard = '\xAB';
		sFindData.lpszSection = ".text";
		sFindData.uMask = WFD_SECTION | WFD_WILDCARD;

		memcpy(sFindData.lpData, "\x6A\x03\x00\x00\x00\x00\x00\xE8\x00\x00\x00\x00\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90", 23);
		memcpy(sFindData.lpData + 2, chMove, 5);
		memcpy(sFindData.lpData + 8, (CHAR *)&uCallB, 4);

		m_dgc->Replace(CBAddDiffData, uOffset + 24, &sFindData);

		delete[] sFindData.lpData;

		uPart = 6;

		ZeroMemory(&sFindData, sizeof(sFindData));
		sFindData.lpData = "80 3D AB AB AB 00 00 0F AB AB AB 00 00 A1 AB AB AB 00 AB AB 0F AB AB 00 00 00 83 F8 12 0F 84 AB 00 00 00";
		sFindData.chWildCard = '\xAB';
		sFindData.lpszSection = ".text";
		sFindData.uMask = WFD_PATTERN | WFD_SECTION | WFD_WILDCARD;

		uOffset = m_dgc->Match(&sFindData);

		uPart = 7;

		ZeroMemory(&sFindData, sizeof(sFindData));
		sFindData.lpData = "\x90\x90\x90\x90\x90\x90";
		sFindData.uDataSize = 6;

		m_dgc->Replace(CBAddDiffData, uOffset + 20, &sFindData);
		m_dgc->Replace(CBAddDiffData, uOffset + 29, &sFindData);

		uPart = 8;

		ZeroMemory(&sFindData, sizeof(sFindData));
		sFindData.lpData = "8B F1 8B 46 04 C7 40 14 00 00 00 00 83 3D AB AB AB 00 0B 75 AB 8B 0D AB AB AB 00 6A 01 6A 00 6A 00 68 AB AB AB 00 68 AB AB AB 00 51 FF 15 AB AB AB 00 C7 06 00 00 00 00";
		sFindData.chWildCard = '\xAB';
		sFindData.uMask = WFD_PATTERN | WFD_WILDCARD;

		uOffset = m_dgc->Match(&sFindData);

		uPart = 9;

		ZeroMemory(&sFindData, sizeof(sFindData));
		sFindData.lpData = "8B 4C E4 10 81 F9 00 00 00 01 77 04 8B 4C E4 0C 52 50 8B 11 8B 42 18 6A 00 6A 00 6A 00 68 1D 27 00 00 C7 41 0C 03 00 00 00 FF D0 58 5A 90 90 90 90 90 90 90 90 90 90 90";
		sFindData.uMask = WFD_PATTERN;

		m_dgc->Replace(CBAddDiffData, uOffset, &sFindData);

	}
	catch (LPCSTR lpszMsg)
	{
		sprintf_s(szMsg, 256, "WDRestoreLoginWindow :: Part %d :: %s", uPart, lpszMsg);
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