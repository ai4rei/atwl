#include <stdlib.h>
#include <crtdbg.h>

#ifdef _DEBUG
	#ifndef DBG_NEW
		#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
		#define new DBG_NEW
	#endif // DEBUG_NEW
#endif  // _DEBUG

#include "WDGRemoveSerialDisplay.h"

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
		TEXT("Remove Serial Display"),
		TEXT("Removes the display of the client serial number in the login window (bottom right corner)."),
		TEXT("[UI]"),
		TEXT(""),
		TEXT("Shinryo"),
		1,
		0,
		{ 0x1481527a, 0x67d3, 0x4d47, { 0x89, 0x6b, 0x5b, 0x43, 0xca, 0xf3, 0x19, 0xb2 } }
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
	if(m_dgc->GetClientDate() <= 20101116)
	{
		return NULL;
	}

	WeeDiffGenPlugin::FINDDATA sFindData = {0};
	CHAR szMsg[256];
	m_diffdata.clear();

	UINT32 uOffset = 0;

	try
	{
		ZeroMemory(&sFindData, sizeof(sFindData));
		sFindData.lpData = "83 C0 AB 3B 41 AB 0F 8C AB 00 00 00 56 6A 00";
		sFindData.chWildCard = '\xAB';
		sFindData.uMask = WFD_PATTERN|WFD_WILDCARD;

		uOffset = m_dgc->Match(&sFindData);
	}
	catch (LPCSTR lpszMsg)
	{
		sprintf_s(szMsg, 256, "WDGRemoveSerialDisplay :: Part 1 :: %s", lpszMsg);
		m_dgc->LogMsg(szMsg);
		return NULL;
	}

	try
	{
		ZeroMemory(&sFindData, sizeof(sFindData));
		sFindData.lpData = "\x31\xC0\x83\xF8\x01\x90";
		sFindData.uDataSize = 6;

		m_dgc->Replace(CBAddDiffData, uOffset, &sFindData);
	} 
	catch (LPCSTR lpszMsg)
	{
		sprintf_s(szMsg, 256, "WDGRemoveSerialDisplay :: Part 2 :: %s", lpszMsg);
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