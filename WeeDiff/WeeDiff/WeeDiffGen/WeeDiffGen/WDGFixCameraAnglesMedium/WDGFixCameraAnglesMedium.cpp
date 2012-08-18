#include <stdlib.h>
#include <crtdbg.h>

#ifdef _DEBUG
	#ifndef DBG_NEW
		#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
		#define new DBG_NEW
	#endif // DEBUG_NEW
#endif  // _DEBUG

#include "WDGFixCameraAnglesMedium.h"

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
		TEXT("Fix Camera Angles Medium"),
		TEXT("Allows you to rotate around the x-axis for almost 60 degrees."),
		TEXT("[UI]"),
		TEXT(""),
		TEXT("Shinryo"),
		1,
		0,
		{ 0x37905b3e, 0x966e, 0x4a83, { 0xa4, 0x45, 0xf, 0xd5, 0x8, 0xcd, 0x6a, 0xf8 } }
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
		FLOAT flAngleValue = 42.00;

		ZeroMemory(&sFindData, sizeof(sFindData));
		sFindData.lpData = "74 AB D9 05 AB AB AB 00 D9 5C 24 08";
		sFindData.chWildCard = '\xAB';
		sFindData.uMask = WFD_PATTERN | WFD_WILDCARD;

		uOffset = m_dgc->Match(&sFindData);
		uOffset += 4;

		uPart = 2;

		UINT32 uFreeOffset = m_dgc->GetNextFreeOffset(4);		

		uPart = 3;		

		ZeroMemory(&sFindData, sizeof(sFindData));
		sFindData.lpData = (CHAR *)&flAngleValue;
		sFindData.uDataSize = 4;

		m_dgc->Replace(CBAddDiffData, uFreeOffset, &sFindData);

		uPart = 4;

		uFreeOffset = m_dgc->Raw2Rva(uFreeOffset);

		ZeroMemory(&sFindData, sizeof(sFindData));
		sFindData.lpData = (CHAR *)&uFreeOffset;
		sFindData.uDataSize = 4;

		m_dgc->Replace(CBAddDiffData, uOffset, &sFindData);
	}
	catch (LPCSTR lpszMsg)
	{
		sprintf_s(szMsg, 256, "WDGFixCameraAnglesMedium :: Part %d :: %s", uPart, lpszMsg);
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