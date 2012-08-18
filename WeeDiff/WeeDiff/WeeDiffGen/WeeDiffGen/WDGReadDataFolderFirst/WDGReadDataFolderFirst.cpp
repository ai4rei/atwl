#include <stdlib.h>
#include <crtdbg.h>

#ifdef _DEBUG
	#ifndef DBG_NEW
		#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
		#define new DBG_NEW
	#endif // DEBUG_NEW
#endif  // _DEBUG

#include "WDGReadDataFolderFirst.h"

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
		TEXT("Read Data Folder First"),
		TEXT("Attempts to read files inside the data folder prior to those in grf archives."),
		TEXT("[Data]"),
		TEXT(""),
		TEXT("Shinryo"),
		1,
		0,
		{ 0x8cc3dbbe, 0x17c7, 0x406a, { 0x9f, 0xef, 0x74, 0x71, 0x4f, 0xd7, 0xe3, 0x5b } },
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
		sFindData.lpData = "'readfolder'";
		sFindData.uMask = WFD_PATTERN;
		UINT32 uOffsetA = m_dgc->FindStr(&sFindData, true);

		ZeroMemory(&sFindData, sizeof(sFindData));
		sFindData.lpData = "'loading'";
		sFindData.uMask = WFD_PATTERN;
		UINT32 uOffsetB = m_dgc->FindStr(&sFindData, true);

		ZeroMemory(&sFindData, sizeof(sFindData));
		sFindData.lpData = new CHAR[28];
		sFindData.uDataSize = 28;
		sFindData.lpszSection = ".text";
		sFindData.chWildCard = '\xAB';
		sFindData.uMask =  WFD_SECTION | WFD_WILDCARD;

		memcpy(sFindData.lpData, "\x68\x00\x00\x00\x00\x8B\xAB\xE8\xAB\xAB\xAB\xAB\x85\xC0\x74\x07\xC6\x05\xAB\xAB\xAB\xAB\x01\x68\x00\x00\x00\x00", 28);
		memcpy(sFindData.lpData + 1, (CHAR *)&uOffsetA, 4);
		memcpy(sFindData.lpData + 24, (CHAR *)&uOffsetB, 4);

		uOffset = m_dgc->Match(&sFindData);

		delete[] sFindData.lpData;
	}
	catch (LPCSTR lpszMsg)
	{
		sprintf_s(szMsg, 256, "WDGReadDataFolderFirst :: Part 1 :: %s", lpszMsg);
		m_dgc->LogMsg(szMsg);
		return NULL;
	}

	try
	{
		ZeroMemory(&sFindData, sizeof(sFindData));
		sFindData.lpData = "\x90\x90";
		sFindData.uDataSize = 2;

		m_dgc->Replace(CBAddDiffData, uOffset + 14, &sFindData);
	} 
	catch (LPCSTR lpszMsg)
	{
		sprintf_s(szMsg, 256, "WDGReadDataFolderFirst :: Part 2 :: %s", lpszMsg);
		m_dgc->LogMsg(szMsg);
		return NULL;
	}

	try
	{
		UINT32 uOffsetC = m_dgc->GetDWORD32(uOffset + 18);

		ZeroMemory(&sFindData, sizeof(sFindData));
		sFindData.lpData = new CHAR[16];
		sFindData.uDataSize = 16;
		sFindData.lpszSection = ".text";
		sFindData.chWildCard = '\xAB';
		sFindData.uMask = WFD_SECTION | WFD_WILDCARD;

		memcpy(sFindData.lpData, "\x80\x3D\x00\x00\x00\x00\x00\x57\xB9\xAB\xAB\xAB\x00\x56\x74\x23", 16);
		memcpy(sFindData.lpData + 2, (CHAR *)&uOffsetC, 4);

		uOffset = m_dgc->Match(&sFindData);

		delete[] sFindData.lpData;
	} 
	catch (LPCSTR lpszMsg)
	{
		sprintf_s(szMsg, 256, "WDGReadDataFolderFirst :: Part 3 :: %s", lpszMsg);
		m_dgc->LogMsg(szMsg);
		return NULL;
	}

	try
	{
		ZeroMemory(&sFindData, sizeof(sFindData));
		sFindData.lpData = "\x90\x90";
		sFindData.uDataSize = 2;

		m_dgc->Replace(CBAddDiffData, uOffset + 14, &sFindData);
	} 
	catch (LPCSTR lpszMsg)
	{
		sprintf_s(szMsg, 256, "WDGReadDataFolderFirst :: Part 4 :: %s", lpszMsg);
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