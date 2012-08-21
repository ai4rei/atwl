#include <stdlib.h>
#include <crtdbg.h>

#ifdef _DEBUG
	#ifndef DBG_NEW
		#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
		#define new DBG_NEW
	#endif // DEBUG_NEW
#endif  // _DEBUG

#include "WDGTranslateClientIntoEnglish.h"

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
		TEXT("Translate Client Into English"),
		TEXT("Translates hardcoded text inside the client into english."),
		TEXT("[UI]"),
		TEXT(""),
		TEXT("Shinryo"),
		1,
		0,
		{ 0x28755721, 0x15a, 0x46f7, { 0x8b, 0xdf, 0xc5, 0x54, 0x99, 0x41, 0x60, 0xdc } },
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

	/************************************************************************/
	/* Translate Delete Time
	/************************************************************************/
	try
	{
		ZeroMemory(&sFindData, sizeof(sFindData));
		sFindData.lpData = "25 64 BF F9 20 25 64 C0 CF 20 25 64 BD C3 20 25 64 BA D0 20 25 64 C3 CA 00";
		sFindData.uMask = WFD_PATTERN;

		uOffset = m_dgc->Match(&sFindData);
	}
	catch (LPCSTR lpszMsg)
	{
		sprintf_s(szMsg, 256, "WDGTranslateClientIntoEnglish :: Translate Delete Time :: Part 1 :: %s", lpszMsg);
		m_dgc->LogMsg(szMsg);
		return NULL;
	}

	try
	{
		ZeroMemory(&sFindData, sizeof(sFindData));
		sFindData.lpData = "'Delete: %d/%d - %d:%d:%d'";
		sFindData.uMask = WFD_PATTERN;

		m_dgc->Replace(CBAddDiffData, uOffset, &sFindData, true);
	} 
	catch (LPCSTR lpszMsg)
	{
		sprintf_s(szMsg, 256, "WDGTranslateClientIntoEnglish :: Translate Delete Time :: Part 2 :: %s", lpszMsg);
		m_dgc->LogMsg(szMsg);
		return NULL;
	}

	try
	{
		ZeroMemory(&sFindData, sizeof(sFindData));
		sFindData.lpData = "25 64 B3 E2 20 25 64 BF F9 20 25 64 C0 CF 20 25 64 BD C3 20 25 64 BA D0 20 25 64 C3 CA 20";
		sFindData.uMask = WFD_PATTERN;

		uOffset = m_dgc->Match(&sFindData);
	}
	catch (LPCSTR lpszMsg)
	{
		sprintf_s(szMsg, 256, "WDGTranslateClientIntoEnglish :: Translate Delete Time :: Part 3 :: %s", lpszMsg);
		m_dgc->LogMsg(szMsg);
		return NULL;
	}

	try
	{
		ZeroMemory(&sFindData, sizeof(sFindData));
		sFindData.lpData = "'%d/%d/%d - %d:%d:%d'";
		sFindData.uMask = WFD_PATTERN;

		m_dgc->Replace(CBAddDiffData, uOffset, &sFindData, true);
	} 
	catch (LPCSTR lpszMsg)
	{
		sprintf_s(szMsg, 256, "WDGTranslateClientIntoEnglish :: Translate Delete Time :: Part 4 :: %s", lpszMsg);
		m_dgc->LogMsg(szMsg);
		return NULL;
	}

	/************************************************************************/
	/* Translate Message Box
	/************************************************************************/
	try
	{
		ZeroMemory(&sFindData, sizeof(sFindData));
		sFindData.lpData = "B8 DE BD C3 C1 F6";
		sFindData.uMask = WFD_PATTERN;

		uOffset = m_dgc->Match(&sFindData);
	}
	catch (LPCSTR lpszMsg)
	{
		sprintf_s(szMsg, 256, "WDGTranslateClientIntoEnglish :: Translate Message Box :: Part 1 :: %s", lpszMsg);
		m_dgc->LogMsg(szMsg);
		return NULL;
	}

	try
	{
		ZeroMemory(&sFindData, sizeof(sFindData));
		sFindData.lpData = "'Message'";
		sFindData.uMask = WFD_PATTERN;

		m_dgc->Replace(CBAddDiffData, uOffset, &sFindData, true);
	} 
	catch (LPCSTR lpszMsg)
	{
		sprintf_s(szMsg, 256, "WDGTranslateClientIntoEnglish :: Translate Message Box :: Part 2 :: %s", lpszMsg);
		m_dgc->LogMsg(szMsg);
		return NULL;
	}

	/************************************************************************/
	/* Translate Character Slot Usage
	/************************************************************************/
	try
	{
		ZeroMemory(&sFindData, sizeof(sFindData));
		sFindData.lpData = "00 28 C4 B3 B8 AF C5 CD 2F C3 D1 20 BD BD B7 D4 29 00";
		sFindData.uMask = WFD_PATTERN;

		uOffset = m_dgc->Match(&sFindData);
	}
	catch (LPCSTR lpszMsg)
	{
		sprintf_s(szMsg, 256, "WDGTranslateClientIntoEnglish :: Translate Character Slot Usage :: Part 1 :: %s", lpszMsg);
		m_dgc->LogMsg(szMsg);
		return NULL;
	}

	try
	{
		ZeroMemory(&sFindData, sizeof(sFindData));
		sFindData.lpData = "'(Used / Total)'";
		sFindData.uMask = WFD_PATTERN;

		m_dgc->Replace(CBAddDiffData, uOffset + 1, &sFindData, true);
	} 
	catch (LPCSTR lpszMsg)
	{
		sprintf_s(szMsg, 256, "WDGTranslateClientIntoEnglish :: Translate Character Slot Usage :: Part 2 :: %s", lpszMsg);
		m_dgc->LogMsg(szMsg);
		return NULL;
	}

	/************************************************************************/
	/* Translate Make Character Window Title
	/************************************************************************/
	try
	{
		ZeroMemory(&sFindData, sizeof(sFindData));
		sFindData.lpData = "C4 C9 B8 AF C5 CD 20 B8 B8 B5 E9 B1 E2 00";
		sFindData.uMask = WFD_PATTERN;

		uOffset = m_dgc->Match(&sFindData);
	}
	catch (LPCSTR lpszMsg)
	{
		sprintf_s(szMsg, 256, "WDGTranslateClientIntoEnglish :: Translate Make Character Window Title :: Part 1 :: %s", lpszMsg);
		m_dgc->LogMsg(szMsg);
		return NULL;
	}

	try
	{
		ZeroMemory(&sFindData, sizeof(sFindData));
		sFindData.lpData = "'Make Character'";
		sFindData.uMask = WFD_PATTERN;

		m_dgc->Replace(CBAddDiffData, uOffset, &sFindData, true);
	} 
	catch (LPCSTR lpszMsg)
	{
		sprintf_s(szMsg, 256, "WDGTranslateClientIntoEnglish :: Translate Make Character Window Title :: Part 2 :: %s", lpszMsg);
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