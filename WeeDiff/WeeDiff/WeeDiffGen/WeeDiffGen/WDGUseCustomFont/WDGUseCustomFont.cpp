// -----------------------------------------------------------------
// WDGUseCustomFont
// (c) 2012 Ai4rei/AN
//
// This work is licensed under a
// Creative Commons BY-NC-SA 3.0 Unported License
// http://creativecommons.org/licenses/by-nc-sa/3.0/
//
// -----------------------------------------------------------------

#include "WDGUseCustomFont.h"

#ifndef _ARRAYSIZE
    #define _ARRAYSIZE(x) (sizeof(x)/sizeof((x)[0]))
#endif  /* _ARRAYSIZE */

#define DEFAULT_CUSTOM_FONT _T("Arial")

static /* const */ WDGPLUGININFO l_PluginInfo =
{
    _T("Use Custom Font"),
    _T("Allows the use of user-defined font for all langtypes. The langtype-specific charset is still being enforced, so if the selected font does not support it, the system falls back to a font that does."),
    _T("[UI]"),
    _T(""),
    _T("Ai4rei/AN"),
    1,
    0,
    { 0x6161b1f1, 0xea19, 0x11e1, { 0xa7, 0x78, 0x0, 0x22, 0x15, 0x64, 0x8a, 0x98 } },  /* guid */
    _T(""),
};

static WDGPlugin* l_lpSelfReference = NULL;

static int CALLBACK CheckFont_P_Cb(CONST LOGFONT* lpLf, CONST TEXTMETRIC* lpTm, DWORD FontType, LPARAM lParam)
{
    bool* lpbSuccess = (bool*)lParam;

    lpbSuccess[0] = true;
    return 0;
}

static bool __stdcall CheckFont(LPCTSTR lpszFontName)
{
    bool bSuccess = false;
    HDC hDC = GetDC(NULL);
    LOGFONT Lf = { 0 };

    Lf.lfCharSet = DEFAULT_CHARSET;
    lstrcpyn(Lf.lfFaceName, lpszFontName, _ARRAYSIZE(Lf.lfFaceName));

    EnumFontFamiliesEx(hDC, &Lf, &CheckFont_P_Cb, (LPARAM)&bSuccess, 0);
    ReleaseDC(NULL, hDC);

    return bSuccess;
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
        this->m_szFontFace[0] = 0;
        this->m_dgc->DisplayInputBox(this->GetPluginInfo()->lpszDiffName, _T("Please input the font name to be used:"), this->m_szFontFace, _ARRAYSIZE(this->m_szFontFace));

        if(!this->m_szFontFace[0])
        {// assume default (Arial)
            break;
        }

        if(CheckFont(this->m_szFontFace))
        {
            break;
        }
        else if(this->m_dgc->DisplayMessageBox(this->GetPluginInfo()->lpszDiffName, _T("This font does not seem to be valid, or at least not installed on your system. Do you want to use it anyway?"), _T(""), MBI_QUESTION, MB_YESNO)==IDYES)
        {
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
    return this->m_szFontFace[0] ? this->m_szFontFace : DEFAULT_CUSTOM_FONT;
}

DiffData* WDGPlugin::GeneratePatch(void)
{
    FINDDATA Fd;
    UINT32 uPart = 0, uOffset, uBOffset, uFOffset = 0, uLen;

    this->m_DiffData.clear();

    if(!this->IsSane())
    {
        return NULL;
    }

    try
    {
        // MISSION: Find the g_ServiceType->FontAddr array and
        // update it's values with the offset of the new font.

        // convert face name
        uPart = 1;
        char szFontFace[LF_FACESIZE];
        if(!WideCharToMultiByte(CP_ACP, 0, this->GetInputValue(), -1, szFontFace, _ARRAYSIZE(szFontFace), NULL, NULL))
        {
            throw "Font name is too long.";
        }

        // find first font of the array
        Fd.uMask = WFD_PATTERN|WFD_SECTION;
        Fd.lpData = "'Gulim'000000";  // korean langtype font, first entry
        Fd.lpszSection = this->IsVC9Image() ? ".rdata" : ".data";

        uPart = 2;
        uBOffset = this->m_dgc->Raw2Rva(this->m_dgc->Match(&Fd));

        // find array by it's first entry
        Fd.uMask = 0;
        Fd.lpData = (CHAR*)&uBOffset;
        Fd.uDataSize = 4;
        Fd.uStart = 0;
        Fd.uFinish = ~0U;

        uPart = 3;
        uOffset = this->m_dgc->Match(&Fd);

        // does this font name already exist in the exe?
        uLen = strlen(szFontFace);
        Fd.uMask = WFD_SECTION;
        Fd.lpData = szFontFace;
        Fd.uDataSize = uLen+1;
        Fd.lpszSection = this->IsVC9Image() ? ".rdata" : ".data";

        uPart = 4;
        if(!this->TryMatch(&Fd, &uFOffset))
        {
            // allocate space for the name
            uFOffset = this->m_dgc->GetNextFreeOffset(uLen+1);

            // paste it
            this->SetBuffer(uFOffset, szFontFace, uLen+1);
        }

        uFOffset = this->m_dgc->Raw2Rva(uFOffset);

        // paste this address until we run out of entries
        uBOffset&= 0xfff00000;  // 0x0065ac78 -> 0x00600000
        uPart = 5;
        do
        {
            this->SetBuffer(uOffset, (CHAR*)&uFOffset, 4);

            uOffset+= 4;
        }
        while((this->m_dgc->GetDWORD32(uOffset)&uBOffset)==uBOffset);  
        // NOTE: this might not be entirely fool-proof, but I do not
        // feel like betting on the fact, that the array always ends
        // with 0x00000081 (CHARSET_HANGUL).
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

void WDGPlugin::SetBuffer(UINT uOffset, CHAR* lpBuffer, UINT32 uSize)
{
    for(UINT32 uIdx = 0; uIdx<uSize; uIdx++)
    {
        this->SetByte(uOffset++, lpBuffer[uIdx]);
    }
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
