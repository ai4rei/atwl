// -----------------------------------------------------------------
// WDGRemoveQuakeSkillEffect
// (c) 2012 Ai4rei/AN
//
// This work is licensed under a
// Creative Commons BY-NC-SA 3.0 Unported License
// http://creativecommons.org/licenses/by-nc-sa/3.0/
//
// -----------------------------------------------------------------

#include "WDGRemoveQuakeSkillEffect.h"

#ifndef _ARRAYSIZE
    #define _ARRAYSIZE(x) (sizeof(x)/sizeof((x)[0]))
#endif  /* _ARRAYSIZE */

static /* const */ WDGPLUGININFO l_PluginInfo =
{
    _T("Remove Quake Skill Effect"),
    _T("Removes all visual shake effects that happen when casting Heaven's Drive, Explosion Spirits and similar skills."),
    _T("[UI]"),
    _T(""),
    _T("Ai4rei/AN"),
    1,
    0,
    { 0x6161b1e3, 0xea19, 0x11e1, { 0xa7, 0x78, 0x0, 0x22, 0x15, 0x64, 0x8a, 0x98 } },  /* guid */
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
    FINDDATA Fd;
    UINT32 uOffset;

    this->m_DiffData.clear();

    if(!this->IsSane())
    {
        return NULL;
    }

    for(;;)
    {// multiple choice
    try
    {
        // MISSION: Find CView::SetQuake and CView::SetQuakeInfo.
        // You are pretty much lost, if you are not able to hunt
        // either of them down, as they are next to each other. One
        // VC6 hint being: Look for PUSH 3E4CCCCDh, PUSH 3E75C28F
        // and PUSH 3F800000h. The next call after these 3 PUSHs is
        // CView::SetQuake, right above it is CView::SetQuakeInfo.
        // VC9 does not push float values like longs, but pull them
        // out of somewhere. The tail of CView::SetQuake can serve
        // for comparison.

        Fd.uMask = WFD_PATTERN|WFD_WILDCARD|WFD_SECTION;
        Fd.chWildCard = '?';
        Fd.lpszSection = ".text";

        // 2006-11-01aSakexe ~ 2007-10-17bSakexe (VC6)
        // NOTE: There's no CView::SetQuakeInfo in these. Also while
        //       this is pretty generic signature, it is unique.
        Fd.lpData =
                  "C3"        // RETN
                  "90909090"  // NOP
                  "55"        // PUSH    EBP
                  "8BEC"      // MOV     EBP,ESP
                  "8B45 08"   // MOV     EAX,[ARG.1]
                  "56"        // PUSH    ESI
                  "8BF1"      // MOV     ESI,ECX
                  "8946 04"   // MOV     DWORD PTR DS:[ESI+4],EAX
                              // CALL    WINMM.timeGetTime
                  ;
        if(this->TryMatch(&Fd, &uOffset))
        {
            // PUSH EBP; MOV EBP,ESP -> RETN 14h
            this->SetByte(uOffset+5, 0xC2);
            this->SetByte(uOffset+6, 0x14);
            this->SetByte(uOffset+7, 0x00);
            break;
        }

        // 2009-06-17aSakexe ~ 2010-08-11bRagexeRE (VC6)
        Fd.lpData =
                  "55"              // PUSH    EBP
                  "8BEC"            // MOV     EBP,ESP
                  "8B45 08"         // MOV     EAX,[ARG.1]
                  "8B55 10"         // MOV     EDX,[ARG.3]
                  "8941 04"         // MOV     DWORD PTR DS:[ECX+4],EAX
                  "8B45 0C"         // MOV     EAX,[ARG.2]
                  "8951 0C"         // MOV     DWORD PTR DS:[ECX+0Ch],EDX
                  "8941 08"         // MOV     DWORD PTR DS:[ECX+8],EAX
                  "5D"              // POP     EBP
                  "C2 0C00"         // RETN    0Ch
                  "90909090909090"  // NOP
                  "55"              // PUSH    EBP
                  "8BEC"            // MOV     EBP,ESP
                  ;
        if(this->TryMatch(&Fd, &uOffset))
        {
            // PUSH EBP; MOV EBP,ESP -> RETN 0Ch
            this->SetByte(uOffset+0, 0xC2);
            this->SetByte(uOffset+1, 0x0C);
            this->SetByte(uOffset+2, 0x00);
            // PUSH EBP; MOV EBP,ESP -> RETN 14h
            this->SetByte(uOffset+32, 0xC2);
            this->SetByte(uOffset+33, 0x14);
            this->SetByte(uOffset+34, 0x00);
            break;
        }

        // 2010-08-18bRagexeRE ~ ? (VC9)
        Fd.lpData =
                  "D94424 04"         // FLD     DWORD PTR SS:[ESP+4]
                  "D959 04"           // FSTP    DWORD PTR DS:[ECX+4]
                  "D94424 0C"         // FLD     DWORD PTR SS:[ESP+0Ch]
                  "D959 0C"           // FSTP    DWORD PTR DS:[ECX+0Ch]
                  "D94424 08"         // FLD     DWORD PTR SS:[ESP+8]
                  "D959 08"           // FSTP    DWORD PTR DS:[ECX+8]
                  "C2 0C00"           // RETN    0Ch
                  "CCCCCCCCCCCCCCCC"  // INT3
                  "8B4424 04"         // MOV     EAX,DWORD PTR SS:[ESP+4]
                  ;
        uOffset = this->m_dgc->Match(&Fd);

        // FLD DWORD PTR SS:[ESP+4] -> RETN 0Ch
        this->SetByte(uOffset+0, 0xC2);
        this->SetByte(uOffset+1, 0x0C);
        this->SetByte(uOffset+2, 0x00);
        // MOV EAX,DWORD PTR SS:[ESP+4] -> RETN 14h
        this->SetByte(uOffset+32, 0xC2);
        this->SetByte(uOffset+33, 0x14);
        this->SetByte(uOffset+34, 0x00);
    }
    catch(const char* lpszThrown)
    {
        char szErrMsg[1024];

        wsprintfA(szErrMsg, __FILE__" :: %s", lpszThrown);
        this->m_dgc->LogMsg(szErrMsg);
    }
    break;
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
