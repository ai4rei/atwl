// -----------------------------------------------------------------
// WDGChatColorSelf
// (c) 2012 Ai4rei/AN
//
// This work is licensed under a
// Creative Commons BY-NC-SA 3.0 Unported License
// http://creativecommons.org/licenses/by-nc-sa/3.0/
//
// -----------------------------------------------------------------

#include "WDGChatColorSelf.h"

#ifndef _ARRAYSIZE
    #define _ARRAYSIZE(x) (sizeof(x)/sizeof((x)[0]))
#endif  /* _ARRAYSIZE */

static /* const */ WDGPLUGININFO l_PluginInfo =
{
    _T("Chat Color Self"),
    _T("Changes the text color of own chat messages."),
    _T("[Color]"),
    _T(""),
    _T("Ai4rei/AN"),
    1,
    0,
    { 0x6161b1ee, 0xea19, 0x11e1, { 0xa7, 0x78, 0x0, 0x22, 0x15, 0x64, 0x8a, 0x98 } },  /* guid */
    _T(""),
};

static WDGPlugin* l_lpSelfReference = NULL;

static inline UINT32 __stdcall ___BSWAP(UINT32 m32)
{
    __asm
    {
        MOV     EAX,m32
        BSWAP   EAX
    }
}

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
    for(;;)
    {
        this->m_szColor[0] = 0;
        this->m_dgc->DisplayInputBox(this->GetPluginInfo()->lpszDiffName, _T("Input new color for own chat text:"), this->m_szColor, _ARRAYSIZE(this->m_szColor));

        if(!this->m_szColor[0])
        {// assume default
            this->m_crChatColor = RGB(0x00,0xff,0x00);
            wsprintf(this->m_szColor, _T("#%02X%02X%02X"), GetRValue(this->m_crChatColor), GetGValue(this->m_crChatColor), GetBValue(this->m_crChatColor));
            break;
        }
        else if(this->m_szColor[0]=='#')
        {
            this->m_crChatColor = ___BSWAP(wcstoul(this->m_szColor+1, NULL, 16)<<8);
            break;
        }
    }
    this->m_dgc->UpdateListView();
    this->GeneratePatch();

    return 0;
}

INT32 WDGPlugin::Disabled(void)
{
    return 0;
}

LPCTSTR WDGPlugin::GetInputValue(void)
{
    return this->m_szColor;
}

DiffData* WDGPlugin::GeneratePatch(void)
{
    FINDDATA Fd;
    UINT32 uOffset, uPart = 0;

    this->m_DiffData.clear();

    if(!this->IsSane())
    {
        return NULL;
    }

    try
    {
        // MISSION: Find CGameMode::Zc_Notify_Playerchat and replace
        // chat color with user-provided color.

        Fd.uMask = WFD_PATTERN|WFD_WILDCARD|WFD_SECTION;
        Fd.lpData =
                  "6A 01"        // PUSH    1
                  "68 00FF0000"  // PUSH    00FF00h  ; green
                  "'?'"          // PUSH    R32
                  "6A 01"        // PUSH    1
                  "B9 '????'"    // MOV     ECX,g_windowMgr
                  "E8 '????'"    // CALL    UIWindowMgr::SendMsg
                  ;
        Fd.chWildCard = '?';
        Fd.lpszSection = ".text";

        uPart   = 1;
        uOffset = this->m_dgc->Match(&Fd)+3;

        this->SetByte(uOffset++, GetRValue(this->m_crChatColor));
        this->SetByte(uOffset++, GetGValue(this->m_crChatColor));
        this->SetByte(uOffset++, GetBValue(this->m_crChatColor));
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
