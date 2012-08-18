#include <stdlib.h>
#include <crtdbg.h>

#ifdef _DEBUG
	#ifndef DBG_NEW
		#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
		#define new DBG_NEW
	#endif // DEBUG_NEW
#endif  // _DEBUG

#include "WDGOnlyFirstLoginBackground.h"

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
		TEXT("Only First Login Background"),
		TEXT("Displays always the first login background."),
		TEXT("[UI]"),
		TEXT(""),
		TEXT("Shinryo"),
		1,
		0,
		{ 0x8cc6e6ab, 0xdb37, 0x42fc, { 0x98, 0xc0, 0x7b, 0x24, 0xf1, 0x67, 0x4c, 0xa } }
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
		sFindData.lpData = "'@유저인터페이스\\T_배경%d-%d.bmp'";
		sFindData.uMask = WFD_PATTERN;

		UINT32 uBG1 = m_dgc->FindStr(&sFindData, true) + 2;

		uPart = 2;

		ZeroMemory(&sFindData, sizeof(sFindData));
		sFindData.lpData = "'유저인터페이스\\T2_배경%d-%d.bmp'";
		sFindData.uMask = WFD_PATTERN;

		UINT32 uBG2 = m_dgc->FindStr(&sFindData, true);

		uPart = 3;

		ZeroMemory(&sFindData, sizeof(sFindData));
		sFindData.lpData = new CHAR[5];
		sFindData.uDataSize = 5;

		sFindData.lpData[0] = '\x68';
		memcpy(sFindData.lpData + 1, (CHAR *)&uBG2, 4);

		m_dgc->Matches(CBAddOffset, &sFindData);

		m_offsets = NULL;
		delete[] sFindData.lpData;

		uPart = 4;

		ZeroMemory(&sFindData, sizeof(sFindData));
		sFindData.lpData = new CHAR[4];
		sFindData.uDataSize = 4;

		for(UINT32 i = 0; i < offsets.size(); i++)
		{
			uOffset = offsets[i];
			memcpy(sFindData.lpData, (CHAR *)&uBG1, 4);

			m_dgc->Replace(CBAddDiffData, uOffset + 1, &sFindData);
		}

		delete[] sFindData.lpData;		
	}
	catch (LPCSTR lpszMsg)
	{
		sprintf_s(szMsg, 256, "WDGOnlyFirstLoginBackground :: Part %d :: %s", uPart, lpszMsg);
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