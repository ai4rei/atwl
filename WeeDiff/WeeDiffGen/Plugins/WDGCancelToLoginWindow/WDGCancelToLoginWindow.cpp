// -----------------------------------------------------------------
// WDGCancelToLoginWindow
// (c) 2013-2014 Neo
//
// <no license information>
//
// -----------------------------------------------------------------

#include "WDGCancelToLoginWindow.h"

#ifndef _ARRAYSIZE
    #define _ARRAYSIZE(x) (sizeof(x)/sizeof((x)[0]))
#endif  /* _ARRAYSIZE */

static /* const */ WDGPLUGININFO l_PluginInfo =
{
    _T("Cancel to Login Window"),
    _T("..."),
    _T("[Fix]"),
    _T(""),
    _T("Neo, Ai4rei"),
    1,
    0,
    { 0x6161b1f8, 0xea19, 0x11e1, { 0xa7, 0x78, 0x0, 0x22, 0x15, 0x64, 0x8a, 0x98 } },  /* guid */
    _T("Recommended"),
};

static WDGPlugin* l_lpSelfReference = NULL;

void WDGPlugin::Release(void)
{
    this->m_DiffData.clear();
    l_lpSelfReference = NULL;
    delete this;
}

void WDGPlugin::Free(LPVOID lpBuf)
{
    delete lpBuf;
}

LPWDGPLUGININFO WDGPlugin::GetPluginInfo(void)
{
    return &l_PluginInfo;
}

INT32 WDGPlugin::Enabled(void)
{
    return 0;
}

INT32 WDGPlugin::Disabled(void)
{
    return 0;
}

LPCTSTR WDGPlugin::GetInputValue(void)
{
    return NULL;
}

DiffData* WDGPlugin::GeneratePatch(void)
{
    FINDDATA Fd;
    UINT32 uOffset, uPart = 0, uDialog, uBlock, uCrawl, uJumpBack, uCallSize;
    UINT32 uDisconnec, uInstanceR;

    this->m_DiffData.clear();

    if(!this->IsSane())
    {
        return NULL;
    }

    try
    {
        // MISSING: Make the result from 'do you want to quit' go
        // for a return to login window rather than client quit.

        // Find korean 메시지 ( 'Message' )
        Fd.uMask = WFD_PATTERN|WFD_SECTION;
        Fd.lpData = "B8DEBDC3C1F60000";  // DWORD-aligned string
        Fd.lpszSection = this->IsVC9Image() ? ".rdata" : ".data";

        uPart = 1;

        uOffset = this->m_dgc->Match(&Fd);

        // Find reference to the string, where it is used for a
        // Yes/No dialog for MSI_DO_YOU_REALLY_WANT_TO_QUIT.
        BYTE cMatchCode1[] =
        {
            0x68, 0x00, 0x00, 0x00, 0x00,    // PUSH    ADDR
            0x6A, 0x00,                      // PUSH    0
            0x6A, 0x00,                      // PUSH    0
            0x6A, 0x01,                      // PUSH    1
            0x6A, 0x02,                      // PUSH    2
            0x6A, 0x11,                      // PUSH    MSI_DO_YOU_REALLY_WANT_TO_QUIT
            0xE8,                            // CALL    MsgStr
        };
        ((UINT32*)&cMatchCode1[1])[0] = this->m_dgc->Raw2Rva(uOffset);

        Fd.uMask = WFD_SECTION;
        Fd.lpData = (char*)cMatchCode1;
        Fd.uDataSize = sizeof(cMatchCode1);
        Fd.lpszSection = ".text";

        uPart = 2;

        if(!this->TryMatch(&Fd, &uOffset))
        {
            // Variant for early VC9 clients.
            BYTE cMatchCode2[] =
            {
                0x68, 0x00, 0x00, 0x00, 0x00,    // PUSH    ADDR
                0x50,                            // PUSH    EAX     ; 0
                0x50,                            // PUSH    EAX     ; 0
                0x6A, 0x01,                      // PUSH    1
                0x6A, 0x02,                      // PUSH    2
                0x6A, 0x11,                      // PUSH    MSI_DO_YOU_REALLY_WANT_TO_QUIT
                0xE8,                            // CALL    MsgStr
            };
            ((UINT32*)&cMatchCode2[1])[0] = this->m_dgc->Raw2Rva(uOffset);

            Fd.uMask = WFD_SECTION;
            Fd.lpData = (char*)cMatchCode2;
            Fd.uDataSize = sizeof(cMatchCode2);
            Fd.lpszSection = ".text";

            uPart = 3;

            uOffset = this->m_dgc->Match(&Fd);
            uDialog = uOffset+32;
        }
        else
        {
            uDialog = uOffset+34;
        }

        // Process CMP EAX,IMM8/IMM32
        if(this->m_dgc->GetBYTE(uDialog)==0x3D /* CMP EAX,IMM32 */)
        {
            uDialog+= 5;
        }
        else
        if(this->m_dgc->GetWORD(uDialog)==0xF883 /* CMP EAX,IMM8 */)
        {
            uDialog+= 3;
        }
        else
        {
            throw "Unexpected CMP variant or instruction.";
        }
        uDialog+= 6;  // JNZ ADDR

        uPart = 4;

        // Check for size parameters (newer clients).
        if(this->m_dgc->GetBYTE(uOffset-5)==0x68 /* PUSH */ && this->m_dgc->GetDWORD32(uOffset-4)==0x118 /* 118h */)
        {
            uOffset-= 7;
        }

        // Find CRagConnection::instanceR and CConnection::Disconnect.
        BYTE cMatchCode3[] =
        {
            0x68,  '?',  '?',  '?', 0x00,        // PUSH    ADDR ("5,01,2600,1832")
            0x51,                                // PUSH    ECX
            0xFF, 0xD0,                          // CALL    EAX
            0x83, 0xC4, 0x08,                    // ADD     ESP,8
            0xE8,  '?',  '?',  '?',  '?',        // CALL    CRagConnection::instanceR
            0x8B, 0xC8,                          // MOV     ECX,EAX
            0xE8,  '?',  '?',  '?',  '?',        // CALL    CConnection::Disconnect
        };

        Fd.uMask = WFD_SECTION|WFD_WILDCARD;
        Fd.lpData = (char*)cMatchCode3;
        Fd.uDataSize = sizeof(cMatchCode3);
        Fd.lpszSection = ".text";
        Fd.chWildCard = '?';

        uPart = 5;

        uBlock = this->m_dgc->Match(&Fd);

        // convert relative offsets to absolute
        uInstanceR = uBlock+16+this->m_dgc->GetDWORD32(uBlock+12);
        uDisconnec = uBlock+23+this->m_dgc->GetDWORD32(uBlock+19);

        // setup new code (part A)
        BYTE cCodeSketch1[] =
        {
            0xE8, 0x00, 0x00, 0x00, 0x00,        // CALL    CRagConnection::instanceR
            0x8B, 0xC8,                          // MOV     ECX,EAX
            0xE8, 0x00, 0x00, 0x00, 0x00,        // CALL    CConnection::Disconnect
            0xEB, 0x00,                          // JMP     SHORT v (past JNZ LONG)
        };
        ((UINT32*)&cCodeSketch1[1])[0] = uInstanceR-(uOffset+5);
        ((UINT32*)&cCodeSketch1[8])[0] = uDisconnec-(uOffset+12);
        ((UINT8*)&cCodeSketch1[13])[0] = uDialog-(uOffset+14);

        uPart = 6;

        this->SetBuffer(uOffset, (char*)cCodeSketch1, sizeof(cCodeSketch1));

        // setup new code (part B)
        // there are various code variants after the uDialog offset:
        // - some have method pointer resolving before (early VC9,
        //   VC10), other after (VC9), and also in mids of the
        //   parameters (VC6).
        // - some push 3 zeros, others 4.
        // - call is either near on EAX or EDX, or from DS:[EDX+CONST]
        // - push 2 is always there in the same encoding, but it
        //   cannot fit 2723h
        //
        // ...
        // i want some disasm module >_>;
        // ...

        // crawl through the instructions to find the CALL, also we
        // assume we can bump into PUSH and MOV instructions only
        uCrawl = uDialog;

        uPart = 7;

        for(;;)
        {
            BYTE uByte = this->m_dgc->GetBYTE(uCrawl);

            if(uByte==0x8B)
            {// MOV     R32,R/M32
                uByte = this->m_dgc->GetBYTE(++uCrawl);

                if(uByte==0x0D)
                {// MOV ECX,DS:[IMM32]
                    uCrawl+= 5;
                }
                else
                if(uByte==0x01)
                {// MOV EAX,DS:[ECX]
                    uCrawl++;
                }
                else
                if(uByte==0x11)
                {// MOV EDX,DS:[ECX]
                    uCrawl++;
                }
                else
                if(uByte==0x50)
                {// MOV EDX,DS:[EAX+IMM8]
                    uCrawl+= 2;
                }
                else
                if(uByte==0x42)
                {// MOV EAX,DS:[EDX+IMM8]
                    uCrawl+= 2;
                }
                else
                {
                    throw "Unexpected register combination or displacement width in MOV.";
                }
            }
            else
            if(uByte==0x6A)
            {// PUSH    IMM8
                uCrawl+= 2;
            }
            else
            if(uByte==0xFF)
            {// CALL    R/M32 (absolute)
                break;
            }
            else
            {
                throw "Unexpected instruction.";
            }
        }

        // remember part A from before? we have some place after it
        // to write into. thus save whatever CALL we are sitting on
        // for later, and jump back.
        uJumpBack = uOffset+sizeof(cCodeSketch1);

        BYTE cCodeSketch2[] =
        {
            0xEB, 0x00,
        };
        ((UINT8*)&cCodeSketch2[1])[0] = uJumpBack-(uCrawl+2);

        uPart = 8;

        this->SetBuffer(uCrawl, (char*)cCodeSketch2, sizeof(cCodeSketch2));

        // in this temporary realm we have to:
        // - POP last PUSH (that is, value 2), since we are not
        //   aware of the registers in use, we cannot actually do
        //   POP, but change ESP.
        // - PUSH 2723h (whose sake we do all this for)
        // - do our backed up CALL, since it comes with and without
        //   displacement, we push an optional NOP after it, so we
        //   do not have to set-up two buffers
        // - jump to the PUSH R32 after the backed up CALL
        BYTE cCodeSketch3[] =
        {
            0x83, 0xC4, 0x04,                // ADD     ESP,4
            0x68, 0x23, 0x27, 0x00, 0x00,    // PUSH    2723h
            0xFF, 0x00, 0x90,                // CALL    R32 + NOP / CALL DS:[R32+IMM8]
            0xEB, 0x00,
        };

        if((this->m_dgc->GetBYTE(uCrawl+1)&0xF0)==0xD0)
        {// CALL NEAR R32
            uCallSize = 2;

            ((UINT8*)&cCodeSketch3[9])[0] = this->m_dgc->GetBYTE(uCrawl+1);
        }
        else
        {// assume displacement form
            uCallSize = 3;

            ((UINT8*)&cCodeSketch3[9])[0] = this->m_dgc->GetBYTE(uCrawl+1);
            ((UINT8*)&cCodeSketch3[10])[0] = this->m_dgc->GetBYTE(uCrawl+2);
        }

        ((UINT8*)&cCodeSketch3[12])[0] = (uCrawl+uCallSize)-(uJumpBack+13);

        uPart = 9;

        this->SetBuffer(uJumpBack, (char*)cCodeSketch3, sizeof(cCodeSketch3));
    }
    catch(const char* lpszThrown)
    {
        char szErrMsg[1024];

        wsprintfA(szErrMsg, __FILE__" :: Part %u :: %s", uPart, lpszThrown);
        this->m_dgc->LogMsg(szErrMsg);

        // clean up diffdata (half diff)
        this->m_DiffData.clear();
    }

    return this->m_DiffData.empty() ? NULL : &this->m_DiffData;
}

DiffData* WDGPlugin::GetDiffData(void)
{
    return this->m_DiffData.empty() ? NULL : &this->m_DiffData;
}

bool WDGPlugin::IsVC9Image(void)
{
    try
    {
        this->m_dgc->FindFunction("_except_handler4_common");  // msvcr90.dll/msvcr100.dll
    }
    catch(const char* lpszThrown)
    {
        return false;

        // unused
        (void)lpszThrown;
    }

    return true;
}

bool WDGPlugin::IsSane(void)
{// ensure that it actually is a valid RO client
#define ISSANEMAGIC "gravity"
    FINDDATA Fd;

    Fd.uMask       = WFD_SECTION;
    Fd.lpData      = ISSANEMAGIC;
    Fd.uDataSize   = sizeof(ISSANEMAGIC);
    Fd.lpszSection = this->IsVC9Image() ? ".rdata" : ".data";

    try
    {
        this->m_dgc->Match(&Fd);
    }
    catch(const char* lpszThrown)
    {
        this->m_dgc->LogMsg(__FILE__" :: Image is not sane.");
        return false;

        // unused
        (void)lpszThrown;
    }

    return true;
#undef ISSANEMAGIC
}

void WDGPlugin::SetByte(UINT32 uOffset, UCHAR uValue)
{
    DIFFDATA Diff = { uOffset, uValue };

    this->m_DiffData.push_back(Diff);
}

void WDGPlugin::SetBuffer(UINT uOffset, CHAR* lpBuffer, UINT32 uSize)
{
    for(UINT32 uIdx = 0; uIdx<uSize; uIdx++)
    {
        this->SetByte(uOffset++, lpBuffer[uIdx]);
    }
}

extern "C" __declspec(dllexport) WeeDiffGenPlugin::IWDGPlugin* InitPlugin(LPVOID lpData, USHORT huWeeDiffMajorVersion, USHORT huWeeDiffMinorVersion)
{
    if(l_lpSelfReference)
    {// double initialization
        DebugBreak();
    }
    else
    {
        l_lpSelfReference = new WDGPlugin(lpData);
    }

    return l_lpSelfReference;
}

bool WDGPlugin::TryMatch(LPFINDDATA lpFd, UINT32* lpuOffset)
{
    try
    {
        lpuOffset[0] = this->m_dgc->Match(lpFd);
    }
    catch(const char* lpszThrown)
    {
        return false;

        // unused
        (void)lpszThrown;
    }

    return true;
}
