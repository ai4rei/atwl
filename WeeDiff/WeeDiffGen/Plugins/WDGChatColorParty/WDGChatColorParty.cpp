// -----------------------------------------------------------------
// WDGChatColorParty
// (c) 2012 Ai4rei/AN
//
// This work is licensed under a
// Creative Commons BY-NC-SA 3.0 Unported License
// http://creativecommons.org/licenses/by-nc-sa/3.0/
//
// -----------------------------------------------------------------

#include "WDGChatColorParty.h"

#ifndef _ARRAYSIZE
    #define _ARRAYSIZE(x) (sizeof(x)/sizeof((x)[0]))
#endif  /* _ARRAYSIZE */

static /* const */ WDGPLUGININFO l_PluginInfo =
{
    _T("Chat Color Party"),
    _T("Changes the text color of party chat messages."),
    _T("[Color]"),
    _T(""),
    _T("Ai4rei/AN"),
    1,
    0,
    { 0x6161b1f0, 0xea19, 0x11e1, { 0xa7, 0x78, 0x0, 0x22, 0x15, 0x64, 0x8a, 0x98 } },  /* guid */
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
        this->m_dgc->DisplayInputBox(this->GetPluginInfo()->lpszDiffName, _T("Input new color for own party chat text:"), this->m_szColor, _ARRAYSIZE(this->m_szColor));

        if(!this->m_szColor[0])
        {// assume default
            this->m_crChatColor[SELF] = RGB(0xff,0xc8,0x00);
            break;
        }
        else if(this->m_szColor[0]=='#')
        {
            this->m_crChatColor[SELF] = ___BSWAP(wcstoul(this->m_szColor+1, NULL, 16)<<8);
            break;
        }
    }

    for(;;)
    {
        this->m_szColor[0] = 0;
        this->m_dgc->DisplayInputBox(this->GetPluginInfo()->lpszDiffName, _T("Input new color for others' party chat text:"), this->m_szColor, _ARRAYSIZE(this->m_szColor));

        if(!this->m_szColor[0])
        {// assume default
            this->m_crChatColor[REST] = RGB(0xff,0xc8,0xc8);
            break;
        }
        else if(this->m_szColor[0]=='#')
        {
            this->m_crChatColor[REST] = ___BSWAP(wcstoul(this->m_szColor+1, NULL, 16)<<8);
            break;
        }
    }

    wsprintf(this->m_szColor, _T("#%02X%02X%02X,#%02X%02X%02X"), GetRValue(this->m_crChatColor[SELF]), GetGValue(this->m_crChatColor[SELF]), GetBValue(this->m_crChatColor[SELF]), GetRValue(this->m_crChatColor[REST]), GetGValue(this->m_crChatColor[REST]), GetBValue(this->m_crChatColor[REST]));
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
        // MISSION: Find CGameMode::Zc_Notify_Chat_Party and replace
        // party chat colors with user-provided colors.

        // own
        Fd.uMask = WFD_PATTERN|WFD_WILDCARD|WFD_SECTION;
        Fd.lpData =
                  "6A 03"        // PUSH    3
                  "68 FFC80000"  // PUSH    00C8FFh  ; orange
                  "'?'"          // PUSH    R32
                  "6A 01"        // PUSH    1
                  ;
        Fd.chWildCard = '?';
        Fd.lpszSection = ".text";

        uPart   = 1;
        uOffset = this->m_dgc->Match(&Fd)+3;

        this->SetByte(uOffset++, GetRValue(this->m_crChatColor[SELF]));
        this->SetByte(uOffset++, GetGValue(this->m_crChatColor[SELF]));
        this->SetByte(uOffset++, GetBValue(this->m_crChatColor[SELF]));

        // others'
        Fd.uMask = WFD_PATTERN|WFD_WILDCARD|WFD_SECTION;
        Fd.lpData =
                  "6A 03"        // PUSH    3
                  "68 FFC8C800"  // PUSH    C8C8FFh  ; rose
                  "'?'"          // PUSH    R32
                  "6A 01"        // PUSH    1
                  ;
        Fd.chWildCard = '?';
        Fd.lpszSection = ".text";

        uPart   = 2;
        uOffset = this->m_dgc->Match(&Fd)+3;

        this->SetByte(uOffset++, GetRValue(this->m_crChatColor[REST]));
        this->SetByte(uOffset++, GetGValue(this->m_crChatColor[REST]));
        this->SetByte(uOffset++, GetBValue(this->m_crChatColor[REST]));
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
