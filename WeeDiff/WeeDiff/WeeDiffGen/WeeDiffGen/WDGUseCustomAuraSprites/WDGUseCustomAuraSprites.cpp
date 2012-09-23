#include <stdlib.h>
#include <crtdbg.h>

#ifdef _DEBUG
	#ifndef DBG_NEW
		#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
		#define new DBG_NEW
	#endif // DEBUG_NEW
#endif  // _DEBUG

#include "WDGUseCustomAuraSprites.h"

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
		TEXT("Use Custom Aura Sprites"),
		TEXT("Loads aurafloat.tga and auraring.bmp for auras instead of ring_blue.tga and pikapika2.bmp."),
		TEXT("[Data]"),
		TEXT(""),
		TEXT("Shinryo"),
		1,
		0,
		{ 0x1f7d1fd9, 0x41cb, 0x400e, { 0x98, 0x6c, 0x49, 0xde, 0xab, 0x2b, 0xbb, 0xfd } }
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
	UINT32 uBOffset = 0;

	try
	{
		// Find RVA for ring_blue.tga
		ZeroMemory(&sFindData, sizeof(sFindData));
		sFindData.lpData = "'effect\\ring_blue.tga'";
		sFindData.uMask = WFD_PATTERN|WFD_SECTION;
		sFindData.lpszSection = ".rdata";
		UINT32 uOffsetA = m_dgc->Raw2Rva(m_dgc->Match(&sFindData));

		uPart = 2;

		// Find RVA for pikapika2.bmp
		ZeroMemory(&sFindData, sizeof(sFindData));
		sFindData.lpData = "'effect\\pikapika2.bmp'";
		sFindData.uMask = WFD_PATTERN|WFD_SECTION;
		sFindData.lpszSection = ".rdata";
		UINT32 uOffsetB = m_dgc->Raw2Rva(m_dgc->Match(&sFindData));

		uPart = 3;

		// Find references to these strings.
		ZeroMemory(&sFindData, sizeof(sFindData));
		sFindData.lpszSection = ".text";
		sFindData.chWildCard = '\xAB';
		sFindData.uMask = WFD_SECTION | WFD_WILDCARD;

		try
		{// pattern for rewritten aura effects (obsoletes EF_LEVEL99*)
			char cMatchPx[] =
					/* 00 */ "\x68\x00\x00\x00\x00"              // PUSH    "effect\ring_blue.tga"
					/* 05 */ "\xFF\x15\xAB\xAB\xAB\xAB"          // CALL    NEAR DWORD PTR DS:[&MSVCP90.std::basic_string<char>::basic_string<char>]
					/* 0B */ "\xAB\xAB\xAB\xAB\xAB\xAB\xAB\xAB"  // MOV     [ARG.17],0      ; either a compiler glitch, or some present from the devs
					/* 13 */ "\xAB\xAB\xAB\xAB\xAB\xAB\xAB\xAB"  // MOV     [ARG.17],-1     ; values vary between clients
					/* 1B */ "\x8B\xCE"                          // MOV     ECX,ESI
					/* 1D */ "\xE8\xAB\xAB\xAB\xAB"              // CALL    ADDR
					/* 22 */ "\x8B\x57\xAB"                      // MOV     EAX,DWORD PTR DS:[EDI+CONST]
					/* 25 */ "\x8B\x44\x24\x14"                  // MOV     EDX,DWORD PTR SS:[ESP+14h]
					/* 29 */ "\x68\xFF\x00\x00\x00"              // PUSH    0FFh
					/* 2E */ "\x6A\x64"                          // PUSH    64h
					/* 30 */ "\x6A\x64"                          // PUSH    64h
					/* 32 */ ;

			// this one is pretty loose, since there is not much
			// usable code to hang onto.
			sFindData.lpData = cMatchPx;
			sFindData.uDataSize = 0x32;
			((UINT32*)&cMatchPx[0x01])[0] = uOffsetA;
			uOffset = m_dgc->Match(&sFindData);

			try
			{
				// same goes for pikapika2.bmp
				char cPushStr[] =
						/* 00 */ "\x68\x00\x00\x00\x00"          // PUSH    "effect\pikapika2.bmp"
						/* 05 */ "\xFF\x15"                      // CALL    NEAR DWORD PTR DS:[&MSVCP90.std::basic_string<char>::basic_string<char>]
						/* 07 */ ;

				uPart = 7;

				sFindData.uMask = WFD_SECTION;
				sFindData.lpData = cPushStr;
				sFindData.uDataSize = 0x07;
				((UINT32*)&cPushStr[0x01])[0] = uOffsetB;
				uBOffset = m_dgc->Match(&sFindData)+1-uOffset;
			}
			catch (LPCSTR lpszMsg)
			{
				sprintf_s(szMsg, 256, "WDGCustomAuraSprites :: Part %d :: %s", uPart, lpszMsg);
				m_dgc->LogMsg(szMsg);
				m_diffdata.clear();
				return NULL;
			}
		}
		catch(LPCSTR)
		{
		try
		{// most common pattern
			char cMatchPx[] =
					/* 00 */ "\x68\x00\x00\x00\x00"  // PUSH    OFFSET "effect\ring_blue.tga"
					/* 05 */ "\x8B\xCE"              // MOV     ECX,ESI
					/* 07 */ "\xE8\xAB\xAB\xAB\xAB"  // CALL    ADDR
					/* 0C */ "\xE9\xAB\xAB\xAB\xAB"  // JMP     ADDR
					/* 11 */ "\xAB"                  // PUSH    R32 (=0)
					/* 12 */ "\x68\x00\x00\x00\x00"  // PUSH    OFFSET "effect\pikapika2.bmp"
					/* 17 */ ;
			sFindData.lpData = cMatchPx;
			sFindData.uDataSize = 0x17;

			((UINT32*)&cMatchPx[0x01])[0] = uOffsetA;
			((UINT32*)&cMatchPx[0x13])[0] = uOffsetB;

			uOffset = m_dgc->Match(&sFindData);
			uBOffset = 0x13;
		}
		catch(LPCSTR)
		{// pattern for clients right after VC9 compiles were introduced
			char cMatchPx[] =
					/* 00 */ "\x68\x00\x00\x00\x00"  // PUSH    OFFSET "effect\ring_blue.tga"
					/* 05 */ "\x8B\xCE"              // MOV     ECX,ESI
					/* 07 */ "\xE8\xAB\xAB\xAB\xAB"  // CALL    ADDR
					/* 0C */ "\xE9\xAB\xAB\xAB\xAB"  // JMP     ADDR
					/* 11 */ "\x6A\x00"              // PUSH    0
					/* 13 */ "\x68\x00\x00\x00\x00"  // PUSH    OFFSET "effect\pikapika2.bmp"
					/* 18 */ ;
			sFindData.lpData = cMatchPx;
			sFindData.uDataSize = 0x18;

			((UINT32*)&cMatchPx[0x01])[0] = uOffsetA;
			((UINT32*)&cMatchPx[0x14])[0] = uOffsetB;

			uOffset = m_dgc->Match(&sFindData);
			uBOffset = 0x14;
		}
		}

		uPart = 4;

		// Find some space for the custom aura sprite names at the
		// beginning of the executable.
		IMAGE_NT_HEADERS sItemNTHeaders;
		m_dgc->GetNTHeaders(&sItemNTHeaders);

		UINT32 uNewOffsetA = sItemNTHeaders.OptionalHeader.ImageBase + 0x380;
		UINT32 uNewOffsetB = sItemNTHeaders.OptionalHeader.ImageBase + 0x380 + 21;

		// Update in-code offsets
		ZeroMemory(&sFindData, sizeof(sFindData));
		sFindData.lpData = (CHAR *)&uNewOffsetA;
		sFindData.uDataSize = 4;
		m_dgc->Replace(CBAddDiffData, uOffset + 1, &sFindData);

		uPart = 5;

		ZeroMemory(&sFindData, sizeof(sFindData));
		sFindData.lpData = (CHAR *)&uNewOffsetB;
		sFindData.uDataSize = 4;
		m_dgc->Replace(CBAddDiffData, uOffset + uBOffset, &sFindData);

		uPart = 6;

		// Save sprite names.
		ZeroMemory(&sFindData, sizeof(sFindData));
		sFindData.lpData = "'effect\\aurafloat.tga' 00 'effect\\auraring.bmp' 00 90";
		sFindData.uMask = WFD_PATTERN;

		m_dgc->Replace(CBAddDiffData, 0x380, &sFindData);
	}
	catch (LPCSTR lpszMsg)
	{
		sprintf_s(szMsg, 256, "WDGCustomAuraSprites :: Part %d :: %s", uPart, lpszMsg);
		m_dgc->LogMsg(szMsg);
		m_diffdata.clear();
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