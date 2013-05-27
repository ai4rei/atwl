#include <stdlib.h>
#include <crtdbg.h>

#ifdef _DEBUG
	#ifndef DBG_NEW
		#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
		#define new DBG_NEW
	#endif // DEBUG_NEW
#endif  // _DEBUG

#include "WDGDisableMultipleWindows.h"

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
		TEXT("Disable Multiple Windows"),
		TEXT("Prevents the client from creating more than one instance on all lang types."),
		TEXT("[Fix]"),
		TEXT(""),
		TEXT("Shinryo/Ai4rei"),
		1,
		0,
		{ 0xe852542b, 0xc20b, 0x47a7, { 0xa1, 0x9c, 0x7a, 0x6a, 0xf4, 0x63, 0xf2, 0x6e } }
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
		try
		{
			ZeroMemory(&sFindData, sizeof(sFindData));
			sFindData.lpData = "E8 AB AB AB FF AB FF 15 AB AB AB 00 A1 AB AB AB 00";
			sFindData.lpszSection = ".text";
			sFindData.chWildCard = '\xAB';
			sFindData.uMask = WFD_PATTERN | WFD_SECTION | WFD_WILDCARD;

			uOffset = m_dgc->Match(&sFindData);
			uOffset += 12;
		}
		catch(LPCSTR)
		{
			try
			{
				ZeroMemory(&sFindData, sizeof(sFindData));
				sFindData.lpData = "E8 AB AB AB FF 6A 00 FF 15 AB AB AB 00 A1 AB AB AB 00";
				sFindData.lpszSection = ".text";
				sFindData.chWildCard = '\xAB';
				sFindData.uMask = WFD_PATTERN | WFD_SECTION | WFD_WILDCARD;

				uOffset = m_dgc->Match(&sFindData);
				uOffset += 13;
			}
			catch(LPCSTR)
			{
				// assume this is a client, where gravity already
				// removed the code that prevented multiple client
				// instances from spawning. the great thing about
				// it is, that we do not have the space to do
				// anything in the area where the code used to be...

				// "Please use the correct client."
				char lpszUseCorCliText[] =
					"\xC1\xA4\xBB\xF3\xC0\xFB\xC0\xCE\x20\xB6\xF3"
					"\xB1\xD7\xB3\xAA\xB7\xCE\xC5\xA9\x20\xC5\xAC"
					"\xB6\xF3\xC0\xCC\xBE\xF0\xC6\xAE\xB8\xA6\x20"
					"\xBD\xC7\xC7\xE0\xBD\xC3\xC4\xD1\x20\xC1\xD6"
					"\xBD\xC3\xB1\xE2\x20\xB9\xD9\xB6\xF8\xB4\xCF"
					"\xB4\xD9\x2E"
					;
				FINDDATA Fd;

				Fd.uMask = WFD_SECTION;
				Fd.lpData = lpszUseCorCliText;
				Fd.uDataSize = sizeof(lpszUseCorCliText);
				Fd.lpszSection = ".rdata";

				uPart = 3;

				uOffset = this->m_dgc->Match(&Fd);

				// Find reference to this string.
				char cPushStr[5];
				cPushStr[0] = 0x68;  // PUSH
				((UINT32*)&cPushStr[1])[0] = this->m_dgc->Raw2Rva(uOffset);

				Fd.uMask = WFD_SECTION;
				Fd.lpData = cPushStr;
				Fd.uDataSize = sizeof(cPushStr);
				Fd.lpszSection = ".text";

				uPart = 4;

				uOffset = this->m_dgc->Match(&Fd);

				// Walk back to find the nearest far call (CoInitialize).
				UINT32 uBegin;

				for(uBegin = uOffset; uBegin>0 && !(this->m_dgc->GetBYTE(uBegin)==0x57 && this->m_dgc->GetWORD(uBegin+1)==0x15FF); uBegin--);
				if(uBegin==0)
				{
					uPart = 5;
					throw "Unable to find CoInitialize.";
				}

				// steal call instruction to insert a pretty far jmp
				UINT32 uStolenCall = this->m_dgc->GetDWORD32(uBegin-4)+this->m_dgc->Raw2Rva(uBegin);

				// set up assembly that takes care of the single
				// instance stuff
				char cMutexCode[] =
				/*00*/  "\xE8\x00\x00\x00\x00"                                          // CALL StolenCall
				/*05*/  "\x56"                                                          // PUSH ESI
				/*06*/  "\x33\xF6"                                                      // XOR ESI,ESI
				/*08*/  "\xE8\x09\x00\x00\x00"                                          // PUSH&JMP
				/*0D*/  "\x4B\x45\x52\x4E\x45\x4C\x33\x32\x00"                          // DB 'KERNEL32',0
				/*16*/  "\xFF\x15\x00\x00\x00\x00"                                      // CALL <&GetModuleHandleA>
				/*1C*/  "\xE8\x0D\x00\x00\x00"                                          // PUSH&JMP
				/*21*/  "\x43\x72\x65\x61\x74\x65\x4D\x75\x74\x65\x78\x41\x00"          // DB 'CreateMutexA',0
				/*2E*/  "\x50"                                                          // PUSH EAX
				/*2F*/  "\xFF\x15\x00\x00\x00\x00"                                      // CALL <&GetProcAddress>
				/*35*/  "\xE8\x0F\x00\x00\x00"                                          // PUSH&JMP
				/*3A*/  "\x47\x6C\x6F\x62\x61\x6C\x5C\x53\x75\x72\x66\x61\x63\x65\x00"  // DB 'Global\Surface',0
				/*49*/  "\x56"                                                          // PUSH ESI
				/*4A*/  "\x56"                                                          // PUSH ESI
				/*4B*/  "\xFF\xD0"                                                      // CALL EAX
				/*4D*/  "\x85\xC0"                                                      // TEST EAX,EAX
				/*4F*/  "\x74\x0F"                                                      // JE lFailed
				/*51*/  "\x56"                                                          // PUSH ESI
				/*52*/  "\x50"                                                          // PUSH EAX
				/*53*/  "\xFF\x15\x00\x00\x00\x00"                                      // CALL <&WaitForSingleObject>
				/*59*/  "\x3D\x02\x01\x00\x00"                                          // CMP EAX,258  ; WAIT_TIMEOUT
				/*5E*/  "\x75\x2F"                                                      // JNZ lSuccess
				/*60*/  "\xE8\x09\x00\x00\x00"                                          // lFailed: PUSH&JMP
				/*65*/  "\x4B\x45\x52\x4E\x45\x4C\x33\x32\x00"                          // DB 'KERNEL32',0
				/*6E*/  "\xFF\x15\x00\x00\x00\x00"                                      // CALL <&GetModuleHandleA>
				/*74*/  "\xE8\x0C\x00\x00\x00"                                          // PUSH&JMP
				/*79*/  "\x45\x78\x69\x74\x50\x72\x6F\x63\x65\x73\x73\x00"              // DB 'ExitProcess',0
				/*85*/  "\x50"                                                          // PUSH EAX
				/*86*/  "\xFF\x15\x00\x00\x00\x00"                                      // CALL <&GetProcAddress>
				/*8C*/  "\x56"                                                          // PUSH ESI
				/*8D*/  "\xFF\xD0"                                                      // CALL EAX
				/*8F*/  "\x5E"                                                          // lSuccess: POP ESI
				/*90*/  "\xE9\x00\x00\x00\x00"                                          // JMP AfterStolenCall
				/*95*/  ;

				// obtain space in the .diff section
				uOffset = this->m_dgc->GetNextFreeOffset(0x95);

				// turn stolen call into a far jmp
				this->SetByte(uBegin-5, 0xE9);

				{// fix up JMP/CALL offsets in the snippet
					UINT32 uRelOffset;

					// far jmp
					uRelOffset = this->m_dgc->Raw2Rva(uOffset)-this->m_dgc->Raw2Rva(uBegin);
					this->SetByte(uBegin-4, ((char*)&uRelOffset)[0]);
					this->SetByte(uBegin-3, ((char*)&uRelOffset)[1]);
					this->SetByte(uBegin-2, ((char*)&uRelOffset)[2]);
					this->SetByte(uBegin-1, ((char*)&uRelOffset)[3]);

					// stolen call
					uRelOffset = uStolenCall-this->m_dgc->Raw2Rva(uOffset+0x05);
					memcpy(&cMutexCode[0x01], &uRelOffset, 4);

					// imports
					uPart = 6;
					uRelOffset = this->m_dgc->FindFunction("GetModuleHandleA");
					memcpy(&cMutexCode[0x18], &uRelOffset, 4);

					uPart = 7;
					uRelOffset = this->m_dgc->FindFunction("GetProcAddress");
					memcpy(&cMutexCode[0x31], &uRelOffset, 4);

					uPart = 8;
					uRelOffset = this->m_dgc->FindFunction("WaitForSingleObject");
					memcpy(&cMutexCode[0x55], &uRelOffset, 4);

					uPart = 9;
					uRelOffset = this->m_dgc->FindFunction("GetModuleHandleA");
					memcpy(&cMutexCode[0x70], &uRelOffset, 4);

					uPart = 10;
					uRelOffset = this->m_dgc->FindFunction("GetProcAddress");
					memcpy(&cMutexCode[0x88], &uRelOffset, 4);

					// return jump
					uRelOffset = this->m_dgc->Raw2Rva(uBegin)-this->m_dgc->Raw2Rva(uOffset+0x95);
					memcpy(&cMutexCode[0x95-4], &uRelOffset, 4);
				}

				// paste assembly
				for(UINT32 i = 0; i<sizeof(cMutexCode); i++)
				{
					this->SetByte(uOffset++, cMutexCode[i]);
				}

				return &m_diffdata;
			}
		}

		uPart = 2;

		ZeroMemory(&sFindData, sizeof(sFindData));
		sFindData.lpData = "\xB8\xFF\xFF\xFF";
		sFindData.uDataSize = 4;

		m_dgc->Replace(CBAddDiffData, uOffset, &sFindData);
	}
	catch (LPCSTR lpszMsg)
	{
		sprintf_s(szMsg, 256, "WDGDisableMultipleWindows :: Part %d :: %s", uPart, lpszMsg);
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

void WDGPlugin::SetByte(UINT32 uOffset, UCHAR uValue)
{
	DIFFDATA Diff = { uOffset, uValue };

	this->m_diffdata.push_back(Diff);
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