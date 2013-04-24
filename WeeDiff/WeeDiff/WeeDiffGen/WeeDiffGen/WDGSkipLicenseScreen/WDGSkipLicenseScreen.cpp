#include <stdlib.h>
#include <crtdbg.h>

#ifdef _DEBUG
	#ifndef DBG_NEW
		#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
		#define new DBG_NEW
	#endif // DEBUG_NEW
#endif  // _DEBUG

#include "WDGSkipLicenseScreen.h"

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
		TEXT("Skip License Screen"),
		TEXT("Jumps directly to the login interface without displaying the license screen."),
		TEXT("[UI]"),
		TEXT(""),
		TEXT("Shinryo/MS"),
		1,
		0,
		{ 0x9c668ef, 0x81f2, 0x4290, { 0x98, 0x61, 0x75, 0xe9, 0x30, 0x6b, 0x58, 0x66 } }
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
		sFindData.lpData = "FF 24 85 AB AB AB 00 8D B3 AB AB 00 00 68 AB AB AB 00";
		sFindData.chWildCard = '\xAB';
		sFindData.uMask = WFD_PATTERN | WFD_WILDCARD;
		
		//uOffset = m_dgc->Match(&sFindData);

		try
		{
			uOffset = m_dgc->Match(&sFindData);
		}
		catch (LPCSTR)
		{			
			ZeroMemory(&sFindData, sizeof(sFindData));
			sFindData.lpData = "FF 24 85 AB AB AB 00 AB AB AB AB AB 00 8D B3 AB AB 00 00 68 AB AB AB 00";
			sFindData.chWildCard = '\xAB';
			sFindData.uMask = WFD_PATTERN | WFD_WILDCARD;
			
			try
			{
				uOffset = m_dgc->Match(&sFindData);
			}
			catch (LPCSTR) //RagExe Special -- MStream
			{
				ZeroMemory(&sFindData, sizeof(sFindData));
				sFindData.lpData = "FF 24 85 AB AB AB 00 8D B3 AB AB 00 00 8B AB 39 2D AB AB AB 00";
				sFindData.chWildCard = '\xAB';
				sFindData.uMask = WFD_PATTERN | WFD_WILDCARD;
				uOffset = m_dgc->Match(&sFindData);
			}
		}	
	} 
	catch (LPCSTR lpszMsg)
	{
		sprintf_s(szMsg, 256, "WDGSkipLicenseScreen :: Part 1 :: %s", lpszMsg);
		m_dgc->LogMsg(szMsg);
		return NULL;
	}

	try
	{
		IMAGE_NT_HEADERS sImageNTHeaders;
		IMAGE_SECTION_HEADER sImageSectionHeader;

		m_dgc->GetNTHeaders(&sImageNTHeaders);
		m_dgc->GetSection(".text", &sImageSectionHeader);

		uOffset = m_dgc->GetDWORD32(uOffset + 3) - sImageNTHeaders.OptionalHeader.ImageBase - (sImageSectionHeader.VirtualAddress - sImageSectionHeader.PointerToRawData);

		UINT32 uData = m_dgc->GetDWORD32(uOffset + 8);

		ZeroMemory(&sFindData, sizeof(sFindData));
		sFindData.lpData = (CHAR *)&uData;
		sFindData.uDataSize = 4;

		m_dgc->Replace(CBAddDiffData, uOffset, &sFindData);
		m_dgc->Replace(CBAddDiffData, uOffset + 4, &sFindData);
	} 
	catch (LPCSTR lpszMsg)
	{
		sprintf_s(szMsg, 256, "WDGSkipLicenseScreen :: Part 2 :: %s", lpszMsg);
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