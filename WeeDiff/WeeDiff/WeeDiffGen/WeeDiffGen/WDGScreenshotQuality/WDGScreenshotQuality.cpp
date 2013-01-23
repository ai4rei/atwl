// -----------------------------------------------------------------
// WDGScreenshotQuality
// (c) 2013 Ai4rei/AN
//
// This work is licensed under a
// Creative Commons BY-NC-SA 3.0 Unported License
// http://creativecommons.org/licenses/by-nc-sa/3.0/
//
// -----------------------------------------------------------------

#include "WDGScreenshotQuality.h"

#ifndef _ARRAYSIZE
    #define _ARRAYSIZE(x) (sizeof(x)/sizeof((x)[0]))
#endif  /* _ARRAYSIZE */

static /* const */ WDGPLUGININFO l_PluginInfo =
{
    _T("Change Screenshot Quality"),
    _T("Allows changing the JPEG quality parameter for screenshots."),
    _T("[UI]"),
    _T(""),
    _T("Ai4rei/AN"),
    1,
    0,
    { 0x6161b1f4, 0xea19, 0x11e1, { 0xa7, 0x78, 0x0, 0x22, 0x15, 0x64, 0x8a, 0x98 } },  /* guid */
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
    bool bFail = false;
    UINT32 uQuality;

    for(;;)
    {
        this->m_szJPEGQuality[0] = 0;
        this->m_dgc->DisplayInputBox(this->GetPluginInfo()->lpszDiffName, bFail ? _T("Please do not enter anything funny. Use a whole number 1~100:") : _T("Input new JPEG quality parameter (1~100):"), this->m_szJPEGQuality, _ARRAYSIZE(this->m_szJPEGQuality));

        if(!this->m_szJPEGQuality[0])
        {// assume default
            lstrcpy(this->m_szJPEGQuality, _T("75"));
            break;
        }

        uQuality = wcstoul(this->m_szJPEGQuality, NULL, 10);

        if(uQuality>0 && uQuality<=100)
        {// clean up
            wsprintf(this->m_szJPEGQuality, _T("%u"), uQuality);
            break;
        }

        bFail = true;
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
    return this->m_szJPEGQuality;
}

DiffData* WDGPlugin::GeneratePatch(void)
{
    FINDDATA Fd;
    UINT8 ucQuality;
    UINT32 uOffset, uPart;

    this->m_DiffData.clear();

    if(!this->IsSane())
    {
        return NULL;
    }

    ucQuality = (UINT8)wcstoul(this->m_szJPEGQuality, NULL, 10);

    try
    {
        // MISSION: Find CRender::SaveJPG and replace unnecessary
        // JPEG_CORE_PROPERTIES::DIBChannels (is assigned it's
        // default value) with JPEG_CORE_PROPERTIES::jquality which
        // defaults to 75.

        try
        {
            uPart = 1;

            // Attempt for VC6 match of JPEG_CORE_PROPERTIES::DIBChannels
            Fd.uMask = WFD_PATTERN|WFD_SECTION;
            Fd.lpData =
                /*00*/"C785 ECB0FFFF 03000000"   // MOV     [LOCAL.5061],3 (MOV DWORD PTR SS:[EBP-4F14],3)  ; DIBChannels
                /*0A*/"C785 F0B0FFFF 02000000"   // MOV     [LOCAL.5060],2 (MOV DWORD PTR SS:[EBP-4F10],2)  ; DIBColor
                /*14*/;
            Fd.lpszSection = ".text";

            uOffset = this->m_dgc->Match(&Fd)+0x02;
        }
        catch(const char*)
        {
            uPart = 2;

            // Attempt for VC9 match of JPEG_CORE_PROPERTIES::DIBChannels
            Fd.uMask = WFD_PATTERN|WFD_SECTION;
            Fd.lpData =
                /*00*/"C74424 70 03000000"   // MOV     DWORD PTR SS:[ESP+70h],3    ; DIBChannels
                /*08*/"C74424 74 02000000"   // MOV     DWORD PTR SS:[ESP+74h],2    ; DIBColor
                /*10*/;
            Fd.lpszSection = ".text";

            uOffset = this->m_dgc->Match(&Fd)+0x01;

            uPart = 3;

            // VC9
            this->SetByte(uOffset++, 0x84);       // MOV DST operand 8 bit -> 32 bit
            uOffset++;
            this->SetByte(uOffset++, 0xAC);       // [ESP+70h] -> [ESP+0ACh]
            this->SetByte(uOffset++, 0x00);
            uOffset+= 2;
            this->SetByte(uOffset++, ucQuality);  // 3 -> ucQuality
            this->SetByte(uOffset++, 0x00);
            this->SetByte(uOffset++, 0x00);
            this->SetByte(uOffset++, 0x00);
            this->SetByte(uOffset++, 0x90);       // NOP filling 
            this->SetByte(uOffset++, 0x90);
            this->SetByte(uOffset++, 0x90);
            this->SetByte(uOffset++, 0x90);
            this->SetByte(uOffset++, 0x90);

            return &this->m_DiffData;
        }

        uPart = 4;

        // VC6
        this->SetByte(uOffset++, 0x28);  // [LOCAL.5061] -> [LOCAL.5046]
        this->SetByte(uOffset++, 0xB1);
        uOffset+= 2;
        this->SetByte(uOffset++, ucQuality);  // 3 -> ucQuality
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
