// -----------------------------------------------------------------
// WDGSharedHeadPalettes
// (c) 2012 Ai4rei/AN
//
// This work is licensed under a
// Creative Commons BY-NC-SA 3.0 Unported License
// http://creativecommons.org/licenses/by-nc-sa/3.0/
//
// -----------------------------------------------------------------

#include "WDGSharedHeadPalettes.h"

#ifndef _ARRAYSIZE
    #define _ARRAYSIZE(x) (sizeof(x)/sizeof((x)[0]))
#endif  /* _ARRAYSIZE */

static /* const */ WDGPLUGININFO l_PluginInfo =
{
    _T("Shared Head Palettes"),
    _T("Makes the client use a single hair palette set (head_%d.pal) for all hair styles."),
    _T("[UI]"),
    _T(""),
    _T("Ai4rei/AN"),
    1,
    0,
    { 0x6161b1e6, 0xea19, 0x11e1, { 0xa7, 0x78, 0x0, 0x22, 0x15, 0x64, 0x8a, 0x98 } },  /* guid */
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
    UINT32 uOffset, uPtr, uPart;

    this->m_DiffData.clear();

    if(!this->IsSane())
    {
        return NULL;
    }

    for(;;)
    {// multiple choice
    try
    {
        // MISSION: Find '赣府\赣府%s_%s_%d.pal', modify it to
        // 'head_%d.pal' and remove the %s' PUSH from CALLs for this
        // string in CSession::GetHeadPaletteName.

        // replace the formatted string
        if(this->IsVC9Image())
        {
            Fd.uMask = WFD_PATTERN|WFD_SECTION;
            Fd.lpData = "B8 D3 B8 AE 5C B8 D3 B8 AE 25 73 5F 25 73 5F 25 64 2E 70 61 6C 00";  // 赣府\赣府%s_%s_%d.pal
            Fd.lpszSection = ".rdata";
        }
        else
        {
            Fd.uMask = WFD_PATTERN|WFD_SECTION;
            Fd.lpData = "B8 D3 B8 AE 5C B8 D3 B8 AE 25 73 25 73 5F 25 64 2E 70 61 6C 00";     // 赣府\赣府%s%s_%d.pal
            Fd.lpszSection = ".data";
        }

        uPart = 1;

        uOffset = this->m_dgc->Match(&Fd);

        uPart = 2;

        uPtr = uOffset+5;
        this->SetByte(uPtr++, 0x68);  // h
        this->SetByte(uPtr++, 0x65);  // e
        this->SetByte(uPtr++, 0x61);  // a
        this->SetByte(uPtr++, 0x64);  // d
        this->SetByte(uPtr++, 0x5F);  // _
        this->SetByte(uPtr++, 0x25);  // %
        this->SetByte(uPtr++, 0x64);  // d
        this->SetByte(uPtr++, 0x2E);  // .
        this->SetByte(uPtr++, 0x70);  // p
        this->SetByte(uPtr++, 0x61);  // al
        this->SetByte(uPtr++, 0x6C);  // l
        this->SetByte(uPtr++, 0x00);  // NUL

        // find reference to the string (PUSH OFFSET)
        char cPushStr[5];
        cPushStr[0] = 0x68;  // PUSH
        ((UINT32*)(&cPushStr[1]))[0] = this->m_dgc->Raw2Rva(uOffset);  // OFFSET

        Fd.uMask = WFD_SECTION;
        Fd.lpData = cPushStr;
        Fd.uDataSize = sizeof(cPushStr);
        Fd.lpszSection = ".text";

        uPart = 3;

        uOffset = this->m_dgc->Match(&Fd);

        // since we unfortunately do not have means to walk command
        // by command, hardcoded relative offsets will have to do
        if(this->IsVC9Image())
        {
            uPart = 4;

            this->VoidPushOrThrow(uOffset-5);

            uPart = 5;

            this->VoidPushOrThrow(uOffset-6);
        }
        else
        {
            uPart = 6;

            this->VoidPushOrThrow(uOffset-1);

            uPart = 7;

            this->VoidPushOrThrow(uOffset-8);
        }

        // adjust stack cleanup
        this->SetByte(uOffset+13,0x0C);  // 14h -> 0Ch (in ADD ESP,x)
    }
    catch(const char* lpszThrown)
    {
        char szErrMsg[1024];

        wsprintfA(szErrMsg, __FILE__" :: Part %u :: %s", uPart, lpszThrown);
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

void WDGPlugin::VoidPushOrThrow(UINT32 uOffset)
{
    unsigned char ucByte = this->m_dgc->GetBYTE(uOffset);

    if(ucByte<0x50 || ucByte>0x57)
    {// not a PUSH R32
        throw "PUSH R32 not found at expected position";
    }

    this->SetByte(uOffset, 0x90);  // NOP
}
