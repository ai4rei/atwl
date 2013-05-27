#include <stdlib.h>
#include <crtdbg.h>

#ifdef _DEBUG
	#ifndef DBG_NEW
		#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
		#define new DBG_NEW
	#endif // DEBUG_NEW
#endif  // _DEBUG

#include "WDGEnableDNSSupport.h"

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
		TEXT("Enable DNS Support"),
		TEXT("Allows the client to read fully-qualified host names inside the clientinfo xml file."),
		TEXT("[Add]"),
		TEXT(""),
		TEXT("Shinryo"),
		1, // Major
		0, // Minor
		{ 0x18bd07c8, 0x77dc, 0x4321, { 0xad, 0x3c, 0x23, 0xce, 0x24, 0xf4, 0x13, 0x9a } },
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
		/************************************************************************/
		/* Find entry .
		/************************************************************************/
		ZeroMemory(&sFindData, sizeof(sFindData));
		sFindData.lpData = "E8 AB AB AB FF 8B C8 E8 AB AB AB FF 50 B9 AB AB AB 00 E8 AB AB AB FF A1";
		sFindData.lpszSection = ".text";
		sFindData.chWildCard = '\xAB';
		sFindData.uMask = WFD_PATTERN | WFD_SECTION | WFD_WILDCARD;		

		uOffset = m_dgc->Match(&sFindData);

		uPart = 2;

		UINT32 uStartupOffset = uOffset;
		UINT32 uUnknownCall = m_dgc->Raw2Rva(uStartupOffset) + m_dgc->GetDWORD32(uStartupOffset + 1) + 5;

		/************************************************************************/
		/* Pre-set new code.
		/************************************************************************/
		UCHAR *pCode = new UCHAR[76];		

		memcpy(
			pCode,
			// Call Unknown Function - Pos = 1
			"\xE8\x00\x00\x00\x00"						// CALL UnknownCall
			"\x60"										// PUSHAD
			// Pointer of old address - Pos = 8
			"\x8B\x35\x00\x00\x00\x00"					// MOV ESI,DWORD PTR DS:[7F8320]            ; ASCII "127.0.0.1"
			"\x56"										// PUSH ESI
			// Call to gethostbyname - Pos = 15
			"\xFF\x15\x00\x00\x00\x00"					// CALL DWORD PTR DS:[<&WS2_32.#52>]
			"\x8B\x48\x0C"								// MOV ECX,DWORD PTR DS:[EAX+0C]
			"\x8B\x11"									// MOV EDX,DWORD PTR DS:[ECX]
			"\x89\xD0"									// MOV EAX,EDX
			"\x0F\xB6\x48\x03"							// MOVZX ECX,BYTE PTR DS:[EAX+3]
			"\x51"										// PUSH ECX
			"\x0F\xB6\x48\x02"							// MOVZX ECX,BYTE PTR DS:[EAX+2]
			"\x51"										// PUSH ECX
			"\x0F\xB6\x48\x01"							// MOVZX ECX,BYTE PTR DS:[EAX+1]
			"\x51"										// PUSH ECX
			"\x0F\xB6\x08"								// MOVZX ECX,BYTE PTR DS:[EAX]
			"\x51"										// PUSH ECX
			// IP scheme offset - Pos = 46
			"\x68\x00\x00\x00\x00"						// PUSH OFFSET 007B001C                     ; ASCII "%d.%d.%d.%d"
			// Pointer to new address Pos = 51
			"\x68\x00\x00\x00\x00"						// PUSH OFFSET 008A077C                     ; ASCII "127.0.0.1"
			// Call to sprintf - Pos = 57
			"\xFF\x15\x00\x00\x00\x00"					// CALL DWORD PTR DS:[<&MSVCR90.sprintf>]
			"\x83\xC4\x18"								// ADD ESP,18
			// Replace old ptr with new ptr
			// Old Ptr - Pos = 66
			// New Ptr - Pos = 70
			"\xC7\x05\x00\x00\x00\x00\x00\x00\x00\x00"	// MOV DWORD PTR DS:[7F8320],OFFSET 008A07C ; ASCII "127.0.0.1"
			"\x61"										// POPAD
			"\xC3"										// RETN
			,
			76);

		UINT32 uFreeOffset = m_dgc->GetNextFreeOffset(76 + 4 + 16);

		uPart = 3;

		/************************************************************************/
		/* Create call to new code offset.
		/************************************************************************/
		ZeroMemory(&sFindData, sizeof(sFindData));
		sFindData.lpData = new CHAR[5];
		sFindData.uDataSize = 5;

		UINT32 uRvaFreeOffset = m_dgc->Raw2Rva(uFreeOffset) - m_dgc->Raw2Rva(uStartupOffset) - 5 + 2 + 16;

		sFindData.lpData[0] = '\xE8';
		memcpy(sFindData.lpData + 1, (CHAR *)&uRvaFreeOffset, 4);

		m_dgc->Replace(CBAddDiffData, uStartupOffset, &sFindData);

		delete[] sFindData.lpData;

		uPart = 4;

		/************************************************************************/
		/* Find old ptr.
		/************************************************************************/
		ZeroMemory(&sFindData, sizeof(sFindData));
		sFindData.lpData = "A3 AB AB AB 00 EB 0F 83 C0 04 A3 AB AB AB 00 EB 05";
		sFindData.lpszSection = ".text";
		sFindData.chWildCard = '\xAB';
		sFindData.uMask = WFD_PATTERN | WFD_SECTION | WFD_WILDCARD;

		uOffset = m_dgc->Match(&sFindData);

		UINT32 uOldPtr = m_dgc->GetDWORD32(uOffset + 1);

		uPart = 5;

		/************************************************************************/
		/* Find gethostbyname().
		/************************************************************************/
		ZeroMemory(&sFindData, sizeof(sFindData));
		sFindData.lpData = "FF 15 AB AB AB 00 85 C0 75 29 8B AB AB AB AB 00";
		sFindData.lpszSection = ".text";
		sFindData.chWildCard = '\xAB';
		sFindData.uMask = WFD_PATTERN | WFD_SECTION | WFD_WILDCARD;

		UINT32 uGethostbyname = 0;
		try
		{
			uOffset = m_dgc->Match(&sFindData);
			uGethostbyname = m_dgc->GetDWORD32(uOffset + 2);
		}
		catch(LPCSTR)
		{
			ZeroMemory(&sFindData, sizeof(sFindData));
			sFindData.lpData = "E8 AB AB AB 00 85 C0 75 35 8B AB AB AB AB 00";
			sFindData.lpszSection = ".text";
			sFindData.chWildCard = '\xAB';
			sFindData.uMask = WFD_PATTERN | WFD_SECTION | WFD_WILDCARD;

			uOffset = m_dgc->Match(&sFindData);

			uOffset = m_dgc->Raw2Rva(uOffset) + m_dgc->GetDWORD32(uOffset + 1) + 5;
			uGethostbyname = m_dgc->GetDWORD32(m_dgc->Rva2Raw(uOffset) + 2);
		}

		uPart = 6;

		UINT32 uSprintf = m_dgc->FindFunction("sprintf");

		uPart = 7;

		ZeroMemory(&sFindData, sizeof(sFindData));
		sFindData.lpData = "'%d.%d.%d.%d'";
		sFindData.uMask = WFD_PATTERN;

		UINT32 uIPScheme = m_dgc->FindStr(&sFindData, TRUE);

		/************************************************************************/
		/* Replace code buffer.
		/************************************************************************/
		uUnknownCall = uUnknownCall - m_dgc->Raw2Rva(uFreeOffset + 2 + 16) - 5;
		uRvaFreeOffset = m_dgc->Raw2Rva(uFreeOffset);
		
		memcpy(pCode + 1, (CHAR *)&uUnknownCall, 4);
		memcpy(pCode + 8, (CHAR *)&uOldPtr, 4);
		memcpy(pCode + 15, (CHAR *)&uGethostbyname, 4);
		memcpy(pCode + 46, (CHAR *)&uIPScheme, 4);
		memcpy(pCode + 51, (CHAR *)&uRvaFreeOffset, 4);
		memcpy(pCode + 57, (CHAR *)&uSprintf, 4);
		memcpy(pCode + 66, (CHAR *)&uOldPtr, 4);
		memcpy(pCode + 70, (CHAR *)&uRvaFreeOffset, 4);

		ZeroMemory(&sFindData, sizeof(sFindData));
		sFindData.lpData = (CHAR *)pCode;
		sFindData.uDataSize = 76;

		m_dgc->Replace(CBAddDiffData, uFreeOffset + 2 + 16, &sFindData);

		delete[] pCode;

		// Calc unknown call - Pos = 1
		// Pointer of old address - Pos = 8
		// Call to gethostbyname - Pos = 15
		// IP scheme offset - Pos = 46
		// Pointer to new address Pos = 51
		// Call to sprintf - Pos = 57
		// Replace old ptr with new ptr
		// Old Ptr - Pos = 66
		// New Ptr - Pos = 70
	}
	catch (LPCSTR lpszMsg)
	{
		sprintf_s(szMsg, 256, "WDGEnableDNSSupport :: Part %d :: %s", uPart, lpszMsg);
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