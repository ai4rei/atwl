#include <stdlib.h>
#include <crtdbg.h>

#ifdef _DEBUG
	#ifndef DBG_NEW
		#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
		#define new DBG_NEW
	#endif // DEBUG_NEW
#endif  // _DEBUG

#include "WDGDisableHShield.h"

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
		TEXT("Disable HShield"),
		TEXT("Prevents AhnLabs HackShield from beeing loaded during client start up."),
		TEXT("[Fix]"),
		TEXT(""),
		TEXT("Shinryo"),
		1,
		0,
		{ 0x29367b42, 0xb3c4, 0x494c, { 0x9a, 0xb1, 0x54, 0x52, 0xf, 0xaa, 0xc, 0xc1 } },
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

	try
	{
		ZeroMemory(&sFindData, sizeof(sFindData));
		sFindData.lpData = "51 83 3D AB AB AB 00 00 74 04 33 C0 59 C3";
		sFindData.chWildCard = '\xAB';
		sFindData.lpszSection = ".text";
		sFindData.uMask = WFD_PATTERN | WFD_WILDCARD | WFD_SECTION;

		uOffset = m_dgc->Match(&sFindData);
	} 
	catch (LPCSTR lpszMsg)
	{
		sprintf_s(szMsg, 256, "WDGDisableHShield :: Part 1 :: %s", lpszMsg);
		m_dgc->LogMsg(szMsg);
		return NULL;
	}

	try
	{
		ZeroMemory(&sFindData, sizeof(sFindData));
		sFindData.lpData = "31 C0 40 90 90 90 90 90 90 90 90";
		sFindData.uMask = WFD_PATTERN;

		m_dgc->Replace(CBAddDiffData, uOffset + 1, &sFindData);
	} 
	catch (LPCSTR lpszMsg)
	{
		sprintf_s(szMsg, 256, "WDGDisableHShield :: Part 2 :: %s", lpszMsg);
		m_dgc->LogMsg(szMsg);
		return NULL;
	}

	IMAGE_SECTION_HEADER sImageSectioHeader = {0};

	try
	{
		ZeroMemory(&sFindData, sizeof(sFindData));
		sFindData.lpData = "'aossdk.dll'";
		sFindData.uMask = WFD_PATTERN;

		uOffset = m_dgc->FindStr(&sFindData, false);
		m_dgc->GetSection(".rdata", &sImageSectioHeader);
	} 
	catch (LPCSTR lpszMsg)
	{
		sprintf_s(szMsg, 256, "WDGDisableHShield :: Part 3 :: %s", lpszMsg);
		m_dgc->LogMsg(szMsg);
		return NULL;
	}

	uOffset += (sImageSectioHeader.VirtualAddress - sImageSectioHeader.PointerToRawData);

	try
	{
		ZeroMemory(&sFindData, sizeof(sFindData));
		sFindData.lpData = new CHAR[17];
		sFindData.uDataSize = 17;
		sFindData.chWildCard = '\xAB';
		sFindData.lpszSection = ".rdata";
		sFindData.uMask = WFD_WILDCARD | WFD_SECTION;

		memcpy(sFindData.lpData, "\x00\xAB\xAB\xAB\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 17);
		memcpy(sFindData.lpData + 13, (CHAR *)&uOffset, 4);

		uOffset = m_dgc->Match(&sFindData);

		delete[] sFindData.lpData;
	} 
	catch (LPCSTR lpszMsg)
	{
		sprintf_s(szMsg, 256, "WDGDisableHShield :: Part 4 :: %s", lpszMsg);
		m_dgc->LogMsg(szMsg);
		return NULL;
	}

	try
	{
		CHAR buffer[19];
		CHAR bufferClear[19] = {0};

		m_dgc->Read(uOffset + 13 + 13 * 16, (UCHAR *)buffer, 19);

		ZeroMemory(&sFindData, sizeof(sFindData));
		sFindData.lpData = bufferClear;
		sFindData.uDataSize = 19;

		m_dgc->Replace(CBAddDiffData, uOffset + 13 + 13 * 16, &sFindData);

		sFindData.lpData = buffer;
		sFindData.uDataSize = 19;

		m_dgc->Replace(CBAddDiffData, uOffset + 1, &sFindData);

	} 
	catch (LPCSTR lpszMsg)
	{
		sprintf_s(szMsg, 256, "WDGDisableHShield :: Part 4 :: %s", lpszMsg);
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