// -----------------------------------------------------------------
// WDGEnable64kHairstyles
// (c) 2012 Ai4rei/AN
//
// This work is licensed under a
// Creative Commons BY-NC-SA 3.0 Unported License
// http://creativecommons.org/licenses/by-nc-sa/3.0/
//
// -----------------------------------------------------------------

#include "WDGEnable64kHairstyles.h"

#ifndef _ARRAYSIZE
    #define _ARRAYSIZE(x) (sizeof(x)/sizeof((x)[0]))
#endif  /* _ARRAYSIZE */

static /* const */ WDGPLUGININFO l_PluginInfo =
{
    _T("Enable 64K hairstyles"),
    _T("Lifts the limit of 27 official hairstyles up to 65535."),
    _T("[UI]"),
    _T(""),
    _T("Ai4rei/AN"),
    1,
    0,
    { 0x6161b1e7, 0xea19, 0x11e1, { 0xa7, 0x78, 0x0, 0x22, 0x15, 0x64, 0x8a, 0x98 } },  /* guid */
    _T(""),
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
    bool bIsNewFormat = false;
    FINDDATA Fd;
    UINT32 uOffset, uBegin, uPart;

    this->m_DiffData.clear();

    if(!this->IsSane())
    {
        return NULL;
    }

    try
    {
        // MISSION: Disable hard-coded hair style table and generate
        // hair style IDs ad-hoc instead. Works only for clients
        // 2011-12-XXaRagexeRE+. While this diff actually allows
        // about 4.29bil of hair styles, due to packet layout
        // limitations the real possible maximum is 65536 (0~65535).

        // Modify format string
        Fd.uMask = WFD_PATTERN|WFD_SECTION;
        Fd.lpData = "C0 CE B0 A3 C1 B7 5C B8 D3 B8 AE C5 EB '\\%s\\%s_%s.%s' 00";  // 인간족\머리통\%s\%s_%s.%s
        Fd.lpszSection = ".rdata";

        uPart = 1;

        uOffset = this->m_dgc->Match(&Fd);

        this->SetByte(uOffset+18, 0x75);  // %s -> %u

        // Find PUSH reference to the string
        char cPushStr[5];
        cPushStr[0] = 0x68;  // PUSH
        ((UINT32*)&cPushStr[1])[0] = this->m_dgc->Raw2Rva(uOffset);

        Fd.uMask = WFD_SECTION;
        Fd.lpData = cPushStr;
        Fd.uDataSize = sizeof(cPushStr);
        Fd.lpszSection = ".text";

        uPart = 2;

        uOffset = this->m_dgc->Match(&Fd);

        // Walk back to find beginning of the function (PUSH -1; PUSH imm32).
        for(uBegin = uOffset; uBegin>0 && !(this->m_dgc->GetBYTE(uBegin)==0x6A && this->m_dgc->GetWORD(uBegin+1)==0x68FF); uBegin--);
        if(uBegin==0)
        {
            uPart = 3;
            throw "Begin of function not found, the function signature has probably changed.";
        }

        // Update the parameter PUSHed to be the hair style ID
        // itself rather than the string obtained from hard-coded
        // table. Note, that this will mess up existing hair-style
        // IDs 0..12. Also the 2nd and 3rd patch block ensures, that
        // ID 0 (invalid) is mapped to 2, as the table would do.
        Fd.uMask = WFD_PATTERN|WFD_WILDCARD;
        Fd.lpData =
                  "8B4C24 '?'"           // MOV     ECX,DWORD PTR SS:[ESP+X]
                  "73 '?'"               // JNB     SHORT ADDR v
                  "8D4C24 '?'"           // LEA     ECX,DWORD PTR SS:[ESP+X]
                  ;
        Fd.chWildCard = '?';
        Fd.uStart = uBegin;
        Fd.uFinish = uOffset;

        uPart = 4;

        uOffset = this->m_dgc->Match(&Fd)+1;

        uPart = 5;

        if(this->m_dgc->GetBYTE(uOffset+2)!=this->m_dgc->GetBYTE(uOffset+8))
        {
            throw "MOV SRC != LEA SRC";
        }

        // MOV     ECX,DWORD PTR SS:[ESP+X]
        this->SetByte(uOffset++, 0x4D);  // -> MOV     ECX,DWORD PTR SS:[EBP]
        this->SetByte(uOffset++, 0x00);
        this->SetByte(uOffset++, 0x90);  // -> NOP
        // JNB     SHORT ADDR v
        this->SetByte(uOffset++, 0x85);  // -> TEST    ECX,ECX
        this->SetByte(uOffset++, 0xC9);
        // LEA     ECX,DWORD PTR SS:[ESP+X]
        this->SetByte(uOffset++, 0x75);  // -> JNZ     SHORT ADDR v
        this->SetByte(uOffset++, 0x02);
        this->SetByte(uOffset++, 0x41);  // -> INC     ECX
        this->SetByte(uOffset++, 0x41);  // -> INC     ECX

        // Void table lookup.
        Fd.uMask = WFD_PATTERN;
        Fd.lpData =
                  "8B45 00"               // MOV     EAX,DWORD PTR SS:[EBP]
                  "8B1481"                // MOV     EDX,DWORD PTR DS:[ECX+EAX*4]
                  ;
        Fd.uStart = uBegin;
        Fd.uFinish = uOffset;

        uPart = 6;

        uOffset = this->m_dgc->Match(&Fd)+4;

        this->SetByte(uOffset++,0x11);   // -> MOV     EDX,DWORD PTR DS:[ECX]
        this->SetByte(uOffset++,0x90);   // -> NOP

        // Lift limit that protects table from invalid access. We
        // keep the < 0 check, since lifting it would not give any
        // benefits.
        Fd.uMask = WFD_PATTERN|WFD_WILDCARD;
        Fd.lpData =
                  "7C 05"                 // JL      SHORT ADDR v
                  "83F8 '?'"              // CMP     EAX,X
                  "7E 07"                 // JLE     SHORT ADDR v
                  "C745 00 0D000000"      // MOV     DWORD PTR SS:[EBP],0Dh
                  ;
        Fd.chWildCard = '?';
        Fd.uStart = uBegin;
        Fd.uFinish = uOffset;

        uPart = 7;

        uOffset = this->m_dgc->Match(&Fd);

        this->SetByte(uOffset+5, 0xEB);
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
        this->m_dgc->FindFunction("_encode_pointer");  // msvcr90.dll
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
