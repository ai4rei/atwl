#include <stdlib.h>
#include <crtdbg.h>

#ifdef _DEBUG
	#ifndef DBG_NEW
		#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
		#define new DBG_NEW
	#endif // DEBUG_NEW
#endif  // _DEBUG

#include "WDGEnableMultipleGRFExtended.h"

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
		TEXT("Enable Multiple GRFs"),
		TEXT("Tells the client to load grf files based on a list within data.ini."),
		TEXT("[Data]"),
		TEXT(""),
		TEXT("Shinryo"),
		1, // Major
		0, // Minor
		{ 0xa083488a, 0x7726, 0x4d53, { 0x9e, 0x15, 0x55, 0x3b, 0xf3, 0xfc, 0x9e, 0x7a } },
		TEXT("Recommended")
	};

	return &wpi;
}

INT32 WDGPlugin::Enabled()
{
	ZeroMemory(m_szValue, MAX_VALUE * sizeof(TCHAR));
	m_dgc->DisplayInputBox(GetPluginInfo()->lpszDiffName, TEXT("Please enter the name for your grf config file:"), m_szValue, MAX_VALUE);
	m_dgc->UpdateListView();
	GeneratePatch();

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

	CHAR szValue[MAX_VALUE];
	ZeroMemory(szValue, MAX_VALUE);

	UINT32 uConvSize = WideCharToMultiByte(CP_ACP, 0, m_szValue, -1, NULL, 0, NULL, NULL);

	if(uConvSize > MAX_VALUE)
	{
		m_dgc->LogMsg("WDGEnableMultipleGRFExtended :: Failed to convert wide character to multibyte!");
		return NULL;
	}
	else
	{
		try
		{
			/************************************************************************/
			/* Find rdata.grf and prevent it from being loaded.
			/************************************************************************/
			ZeroMemory(&sFindData, sizeof(sFindData));
			sFindData.lpData = "'rdata.grf'";
			sFindData.uMask = WFD_PATTERN;

			uOffset = m_dgc->FindStr(&sFindData);

			ZeroMemory(&sFindData, sizeof(sFindData));
			sFindData.lpData = "\x00\x00\x00\x00\x00\x00\x00\x00\x00";
			sFindData.uDataSize = 9;

			m_dgc->Replace(CBAddDiffData, uOffset, &sFindData);

			uPart = 2;

			/************************************************************************/
			/* Get address to the function that actually adds a grf into the list.
			/************************************************************************/
			ZeroMemory(&sFindData, sizeof(sFindData));
			sFindData.lpData = "'data.grf'";
			sFindData.uMask = WFD_PATTERN;

			UINT32 uGRF = m_dgc->FindStr(&sFindData, true);

			ZeroMemory(&sFindData, sizeof(sFindData));
			sFindData.lpData = new CHAR[21];
			sFindData.uDataSize = 21;
			sFindData.lpszSection = ".text";
			sFindData.chWildCard = '\xAB';
			sFindData.uMask = WFD_SECTION | WFD_WILDCARD;

			memcpy(sFindData.lpData, "\x68\x00\x00\x00\x00\xB9\xAB\xAB\xAB\x00\xE8\xAB\xAB\xAB\xAB\x8B\xAB\xAB\xAB\xAB\x00", 21);
			memcpy(sFindData.lpData + 1, (CHAR *)&uGRF, 4);

			uOffset = m_dgc->Match(&sFindData);

			delete[] sFindData.lpData;

			uPart = 3;

			/************************************************************************/
			/* Save 'this' pointer and the address of AddPak()
			/************************************************************************/

			UCHAR chSetECX[5];
			m_dgc->Read(uOffset + 5, chSetECX, 5);
			UINT32 uAddPak = m_dgc->Raw2Rva(uOffset + 10) + m_dgc->GetDWORD32(uOffset + 11) + 5;

			uPart = 4;

			/************************************************************************/
			/* Initialize code to be inserted.
			/************************************************************************/
			UCHAR *pCode = new UCHAR[247];

			memcpy(pCode,			
				"\xC8\x80\x00\x00"                                        // ENTER   80h, 0
				"\x60"                                                    // PUSHA
				// ST04 = Pos 6
				"\x68\x00\x00\x00\x00"                                    // PUSH OFFSET <ModuleName> ; "KERNEL32"
				// CA00 = Pos 12
				"\xFF\x15\x00\x00\x00\x00"                                // CALL DS:GetModuleHandleA
				"\x85\xC0"                                                // TEST EAX,EAX
				"\x74\x23"                                                // JZ SHORT <distance>
				// CA01 = Pos 22
				"\x8B\x3D\x00\x00\x00\x00"                                // MOV EDI, DS:GetProcAddress
				// ST01 = Pos 27
				"\x68\x00\x00\x00\x00"                                    // PUSH OFFSET <FunctionName> ; "GetPrivateProfileStringA"
				"\x89\xC3"                                                // MOV EBX, EAX
				"\x50"                                                    // PUSH EAX ; hModue
				"\xFF\xD7"                                                // CALL EDI ; GetProcAddress()
				"\x85\xC0"                                                // TEST EAX, EAX
				"\x74\x0F"                                                // JZ SHORT <distance>
				"\x89\x45\xF6"                                            // MOV [EBP+<var>], EAX
				// ST02 = Pos 44
				"\x68\x00\x00\x00\x00"                                    // PUSH OFFSET <FunctionName> ; "WritePrivateProfileStringA"
				"\x89\xD8"                                                // MOV EAX, EBX
				"\x50"                                                    // PUSH EAX ; hModule
				"\xFF\xD7"                                                // CALL EDI ; GetProcAddress()
				"\x85\xC0"                                                // TEST EAX, EAX
				"\x74\x6E"                                                // JZ SHORT <distance>
				"\x89\x45\xFA"                                            // MOV [EBP+<var>], EAX
				"\x31\xD2"                                                // XOR EDX, EDX
				"\x66\xC7\x45\xFE\x39\x00"                                // MOV [EBP+<var>], 39h ; Char 9
				"\x52"                                                    // PUSH EDX
				// ST00 = Pos 70
				"\x68\x00\x00\x00\x00"                                    // PUSH OFFSET <StringOffset> ; ".\\DATA.INI"
				"\x6A\x74"                                                // PUSH 74h
				"\x8D\x5D\x81"                                            // LEA EBX, [EBP+<var>]
				"\x53"                                                    // PUSH EBX
				"\x8D\x45\xFE"                                            // LEA EAX, [EBP+<var>]
				"\x50"                                                    // PUSH EAX
				"\x50"                                                    // PUSH EAX
				// ST03 = Pos 86
				"\x68\x00\x00\x00\x00"                                    // PUSH OFFSET <StringOffset>  ; "Data"
				"\xFF\x55\xF6"                                            // CALL [EBP+<var>]
				"\x8D\x4D\xFE"                                            // LEA ECX, [EBP+<var>]
				"\x66\x8B\x09"                                            // MOV CX, [ECX]
				"\x8D\x5D\x81"                                            // LEA EBX, [EBP+<var>]
				"\x66\x3B\x0B"                                            // CMP CX, [EBX]
				"\x5A"                                                    // POP EDX
				"\x74\x0E"                                                // JZ SHORT <distance>
				"\x52"                                                    // PUSH EDX
				"\x53"                                                    // PUSH EBX
				// SETECX = Pos 110
				"\x00\x00\x00\x00\x00"                                    // MOV ECX, OFFSET <this pointer>
				// CA02 = Pos 116
				"\xE8\x00\x00\x00\x00"                                    // CALL CFileMgr::AddPak()
				"\x5A"                                                    // POP EDX
				"\x42"                                                    // INC EDX
				"\xFE\x4D\xFE"                                            // DEC BYTE PTR [EBP+<var>]
				"\x80\x7D\xFE\x30"                                        // CMP BYTE PTR [EBP+<var>], 30h ; Char 0
				"\x73\xC1"                                                // JNB SHORT <distance>
				"\x85\xD2"                                                // TEST EDX, EDX
				"\x75\x20"                                                // JNZ SHORT <distance>
				// ST00 = Pos 136
				"\x68\x00\x00\x00\x00"                                    // PUSH OFFSET <StringOffset> ; ".\\DATA.INI"
				// uGRF = 141
				"\x68\x00\x00\x00\x00"                                    // PUSH OFFSET <StringOffset> ; "data.grf"
				"\x66\xC7\x45\xFE\x32\x00"                                // MOV [EBP+<var>], 32h
				"\x8D\x45\xFE"                                            // LEA EAX, [EBP+<var>]
				"\x50"                                                    // PUSH EAX
				// ST03 = Pos 156
				"\x68\x00\x00\x00\x00"                                    // PUSH OFFSET <StringOffset>  ; "Data"
				"\xFF\x55\xFA"                                            // CALL [EBP+<var>]
				"\x85\xC0"                                                // TEST EAX, EAX
				"\x75\x97"                                                // JNZ SHORT <distance>
				"\x61"                                                    // POPA
				"\xC9"                                                    // LEAVE
				"\xC3",                                                   // RETN
				170
				);

			UINT32 uFreeOffset = m_dgc->GetNextFreeOffset(247 + 4);

			uPart = 5;

			ZeroMemory(&sFindData, sizeof(sFindData));
			sFindData.lpData = new CHAR[15];
			sFindData.uDataSize = 15;

			UINT32 uRvaFreeOffset = m_dgc->Raw2Rva(uFreeOffset) - m_dgc->Raw2Rva(uOffset) - 5;

			memcpy(sFindData.lpData, "\xE8\x00\x00\x00\x00\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90", 15);
			memcpy(sFindData.lpData + 1, (CHAR *)&uRvaFreeOffset, 4);

			m_dgc->Replace(CBAddDiffData, uOffset, &sFindData);

			delete[] sFindData.lpData;

			uPart = 6;

			CHAR szStrings[] = 
				// 0; Size: 11;
				".\\DATA.INI\x00"
				// 11; Size: 25;
				"GetPrivateProfileStringA\x00"
				// 36; Size: 27;
				"WritePrivateProfileStringA\x00"
				// 63; Size: 5;
				"Data\x00"
				// 68; Size: 9;
				"KERNEL32\x00"
				;

			UINT32 uCA00 = m_dgc->FindFunction("GetModuleHandleA");
			UINT32 uCA01 = m_dgc->FindFunction("GetProcAddress");
			UINT32 uCA02 = uAddPak - m_dgc->Raw2Rva(uFreeOffset + 115) - 5;
			UINT32 uST00 = m_dgc->Raw2Rva(uFreeOffset) + 170;
			UINT32 uST01 = uST00 + 11;
			UINT32 uST02 = uST01 + 25;
			UINT32 uST03 = uST02 + 27;
			UINT32 uST04 = uST03 + 5;

			memcpy(pCode + 141, (CHAR *)&uGRF, 4);
			memcpy(pCode + 110, chSetECX, 5);
			memcpy(pCode +  12, (CHAR *)&uCA00, 4);
			memcpy(pCode +  22, (CHAR *)&uCA01, 4);
			memcpy(pCode + 116, (CHAR *)&uCA02, 4);		
			memcpy(pCode +  70, (CHAR *)&uST00, 4);
			memcpy(pCode + 136, (CHAR *)&uST00, 4);
			memcpy(pCode +  27, (CHAR *)&uST01, 4);
			memcpy(pCode +  44, (CHAR *)&uST02, 4);
			memcpy(pCode +  86, (CHAR *)&uST03, 4);
			memcpy(pCode + 156, (CHAR *)&uST03, 4);
			memcpy(pCode +   6, (CHAR *)&uST04, 4);
			memcpy(pCode + 170, szStrings, 77);

			ZeroMemory(&sFindData, sizeof(sFindData));
			sFindData.lpData = (CHAR *)pCode;
			sFindData.uDataSize = 247;

			m_dgc->Replace(CBAddDiffData, uFreeOffset, &sFindData);

			delete[] pCode;
		}
		catch (LPCSTR lpszMsg)
		{
			sprintf_s(szMsg, 256, "WDGEnableMultipleGRFExtended :: Part %d :: %s", uPart, lpszMsg);
			m_dgc->LogMsg(szMsg);
			return NULL;
		}
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