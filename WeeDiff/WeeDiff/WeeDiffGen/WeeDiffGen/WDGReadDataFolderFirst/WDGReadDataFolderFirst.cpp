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

	UINT32 uOffset = 0, uPart = 0;

	try
	{
		uPart = 1;
		sFindData.uMask = WFD_PATTERN|WFD_SECTION;
		sFindData.lpData = "'readfolder' 00";
		sFindData.lpszSection = ".rdata";
		UINT32 uOffsetA = m_dgc->Raw2Rva(m_dgc->Match(&sFindData));

		uPart = 2;
		sFindData.uMask = WFD_PATTERN|WFD_SECTION;
		sFindData.lpData = "'loading' 00";
		sFindData.lpszSection = ".rdata";
		UINT32 uOffsetB = m_dgc->Raw2Rva(m_dgc->Match(&sFindData));

		char cPushStr[] =
				/* 00 */ "\x68\x00\x00\x00\x00"          // PUSH    "readfolder"
				/* 05 */ "\x8B\xCD"                      // MOV     ECX,EBP
				/* 07 */ "\xE8\xAB\xAB\xAB\xAB"          // CALL    XMLElement::FindChild
				/* 0C */ "\x85\xC0"                      // TEST    EAX,EAX
				/* 0E */ "\x74\xAB"                      // JE      SHORT ADDR v
				/* 10 */ "\xC6\x05\xAB\xAB\xAB\xAB\x01"  // MOV     BYTE PTR DS:[g_readFolderFirst],1h
				/* 17 */ "\x68\x00\x00\x00\x00"          // PUSH    "loading"
				/* 1C */ ;
		uPart = 3;
		sFindData.uMask = WFD_SECTION|WFD_WILDCARD;
		sFindData.lpData = cPushStr;
		sFindData.uDataSize = 0x1C;
		sFindData.chWildCard = '\xAB';
		sFindData.lpszSection = ".text";

		((UINT32*)&cPushStr[0x01])[0] = uOffsetA;
		((UINT32*)&cPushStr[0x18])[0] = uOffsetB;

		uOffset = m_dgc->Match(&sFindData);

		// fetch g_readFolderFirst address
		UINT32 uFolderFirstOffset = m_dgc->GetDWORD32(uOffset+0x12);

		// find read-reference to g_readFolderFirst
		char cReadRef[7] = { 0 };
		((UINT16*)&cReadRef[0])[0] = 0x3D80;  // CMP
		((UINT32*)&cReadRef[2])[0] = uFolderFirstOffset;
		uPart = 4;
		sFindData.uMask = WFD_SECTION;
		sFindData.lpData = cReadRef;
		sFindData.uDataSize = sizeof(cReadRef);
		sFindData.lpszSection = ".text";

		uOffset = m_dgc->Match(&sFindData);

		// transform to a sequence that let's the code think, that
		// g_readFolderFirst is set. this won't even break when the
		// flag is set by clientinfo.xml
		this->SetByte(uOffset+0,0x90);   // NOP
		this->SetByte(uOffset+1,0xA1);   // MOV     EAX,DWORD PTR DS:[g_readFolderFirst]
		this->SetByte(uOffset+6,0x40);   // INC     EAX
	}
	catch (LPCSTR lpszMsg)
	{
		sprintf_s(szMsg, 256, "WDGReadDataFolderFirst :: Part %u :: %s", uPart, lpszMsg);
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