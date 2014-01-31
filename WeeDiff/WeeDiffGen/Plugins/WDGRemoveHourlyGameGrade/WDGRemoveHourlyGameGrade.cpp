// -----------------------------------------------------------------
// WDGRemoveHourlyGameGrade
// (c) 2012 Ai4rei/AN
//
// This work is licensed under a
// Creative Commons BY-NC-SA 3.0 Unported License
// http://creativecommons.org/licenses/by-nc-sa/3.0/
//
// -----------------------------------------------------------------

#include "WDGRemoveHourlyGameGrade.h"

#ifndef _ARRAYSIZE
    #define _ARRAYSIZE(x) (sizeof(x)/sizeof((x)[0]))
#endif  /* _ARRAYSIZE */

static /* const */ WDGPLUGININFO l_PluginInfo =
{
    _T("Remove Hourly Game Grade Banner"),
    _T("Removes age appropriance banner that appears for 10 seconds every hour at o'clock to the left of the mini-map."),
    _T("[UI]"),
    _T(""),
    _T("Ai4rei/AN"),
    1,
    0,
    { 0x6161b1e2, 0xea19, 0x11e1, { 0xa7, 0x78, 0x0, 0x22, 0x15, 0x64, 0x8a, 0x98 } },  /* guid */
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
        // MISSION: Find a sequence of 3 JNZ at the beginning of the
        // first function invoked by CGameMode::DrawTime.

        Fd.uMask = WFD_PATTERN|WFD_WILDCARD|WFD_SECTION;
        Fd.chWildCard = '?';
        Fd.lpszSection = ".text";

        // 2010-03-17cRagexeRE ~ 2010-08-10aRagexeRE (VC6)
        Fd.lpData =
                  "75 32"    // JNZ     SHORT ADDR v
                  "8B45 B8"  // MOV     EAX,[LOCAL.18]
                  "66 85C0"  // TEST    AX,AX
                  "75 15"    // JNZ     SHORT ADDR v
                  "84C9"     // TEST    CL,CL
                  "75 26"    // JNZ     SHORT ADDR v
                  "B1 01"    // MOV     CL,1
                  "33C0"     // XOR     EAX,EAX
                  ;
        if(this->TryMatch(&Fd, &uOffset))
        {
            this->SetByte(uOffset, 0xEB);  // JNZ -> JMP
            break;
        }

        // 2010-08-18bRagexeRE ~ ? (VC9)
        Fd.lpData =
                  "75 34"          // JNZ     SHORT ADDR v
                  "66 8B4424 '?'"  // MOV     AX,WORD PTR SS:[ESP+?]
                  "66 85C0"        // TEST    AX,AX
                  "75 15"          // JNZ     SHORT ADDR v
                  "84C9"           // TEST    CL,CL
                  "75 26"          // JNZ     SHORT ADDR v
                  "B1 01"          // MOV     CL,1
                  "33C0"           // XOR     EAX,EAX
                  ;
        uOffset = this->m_dgc->Match(&Fd);

        this->SetByte(uOffset, 0xEB);  // JNZ -> JMP
    }
    catch(const char* lpszThrown)
    {
        char szErrMsg[1024];

        wsprintfA(szErrMsg, __FILE__" :: %s", lpszThrown);
        this->m_dgc->LogMsg(szErrMsg);

        // clean up diffdata (half diff)
        this->m_DiffData.clear();
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
