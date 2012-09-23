// -----------------------------------------------------------------
// WDGExtendedNPCDialog
// (c) 2012 Ai4rei/AN
//
// This work is licensed under a
// Creative Commons BY-NC-SA 3.0 Unported License
// http://creativecommons.org/licenses/by-nc-sa/3.0/
//
// -----------------------------------------------------------------

#include "WDGExtendedNPCDialog.h"

#ifndef _ARRAYSIZE
    #define _ARRAYSIZE(x) (sizeof(x)/sizeof((x)[0]))
#endif  /* _ARRAYSIZE */

static /* const */ WDGPLUGININFO l_PluginInfo =
{
    _T("Extended NPC Dialog"),
    _T("Increases max input chars of NPC dialog boxes from 2052 to a value of choice."),
    _T("[UI]"),
    _T(""),
    _T("Ai4rei/AN"),
    1,
    0,
    { 0x6161b1e9, 0xea19, 0x11e1, { 0xa7, 0x78, 0x0, 0x22, 0x15, 0x64, 0x8a, 0x98 } },  /* guid */
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
    UINT32 uSize;

    for(;;)
    {
        this->m_szBufferSize[0] = 0;
        this->m_dgc->DisplayInputBox(this->GetPluginInfo()->lpszDiffName, bFail ? _T("Please do not enter anything funny. Use a whole positive number:") : _T("Input new size of the NPC dialog buffer:"), this->m_szBufferSize, _ARRAYSIZE(this->m_szBufferSize));

        if(!this->m_szBufferSize[0])
        {// assume default (double size)
            wsprintf(this->m_szBufferSize, _T("%u"), 4096);
            break;
        }

        uSize = wcstoul(this->m_szBufferSize, NULL, 10);

        if(uSize)
        {// clean up
            uSize+= (uSize&3) ? 4-(uSize&3) : 0;  // round up to a multiple of 4 to keep the stack pointer aligned
            wsprintf(this->m_szBufferSize, _T("%u"), uSize);
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
    return this->m_szBufferSize;
}

DiffData* WDGPlugin::GeneratePatch(void)
{
    FINDDATA Fd;
    UINT32 uOffset, uBOffset, uPart, uBufferSize, uValue;

    this->m_DiffData.clear();

    if(!this->IsSane())
    {
        return NULL;
    }

    uBufferSize = wcstoul(this->m_szBufferSize, NULL, 10);

    try
    {
        // MISSION: Find CGameMode::Zc_Say_Dialog's stack allocation
        // and modify it to user's needs (should we tell them to
        // keep their desires within sane bounds [2048~8192]?).

        try
        {
            uPart = 1;

            // VC9 stack reserve
            Fd.uMask = WFD_PATTERN|WFD_WILDCARD|WFD_SECTION;
            Fd.lpData =
                /*00*/"81EC 08080000"    // SUB     ESP,808h
                /*06*/"A1 '????'"        // MOV     EAX,DWORD PTR DS:[___security_cookie]
                /*0B*/"33C4"             // XOR     EAX,ESP
                /*0D*/"898424 04080000"  // MOV     DWORD PTR SS:[ESP+804h],EAX
                /*14*/;
            Fd.chWildCard = '?';
            Fd.lpszSection = ".text";

            uOffset = this->m_dgc->Match(&Fd);
        }
        catch(const char*)
        {
            uPart = 2;

            // VC6 stack reserve
            Fd.uMask = WFD_PATTERN|WFD_SECTION;
            Fd.lpData =
                /*00*/"55"               // PUSH    EBP
                /*01*/"8BEC"             // MOV     EBP,ESP
                /*03*/"81EC 04080000"    // SUB     ESP,804h
                /*09*/;
            Fd.lpszSection = ".text";

            uOffset = this->m_dgc->Match(&Fd)+0x05;

            // change the stack reserve
            this->SetByte(uOffset++, ((UCHAR*)&uBufferSize)[0]);
            this->SetByte(uOffset++, ((UCHAR*)&uBufferSize)[1]);
            this->SetByte(uOffset++, ((UCHAR*)&uBufferSize)[2]);
            this->SetByte(uOffset++, ((UCHAR*)&uBufferSize)[3]);

            // nothing more to do, stack frame adjusts itself
            return &this->m_DiffData;
        }

        uValue = uBufferSize+4;  // account for security cookie

        // change the stack reserve
        uBOffset = uOffset+0x02;
        this->SetByte(uBOffset++, ((UCHAR*)&uValue)[0]);
        this->SetByte(uBOffset++, ((UCHAR*)&uValue)[1]);
        this->SetByte(uBOffset++, ((UCHAR*)&uValue)[2]);
        this->SetByte(uBOffset++, ((UCHAR*)&uValue)[3]);

        // update cookie variable
        uBOffset = uOffset+0x10;
        this->SetByte(uBOffset++, ((UCHAR*)&uBufferSize)[0]);
        this->SetByte(uBOffset++, ((UCHAR*)&uBufferSize)[1]);
        this->SetByte(uBOffset++, ((UCHAR*)&uBufferSize)[2]);
        this->SetByte(uBOffset++, ((UCHAR*)&uBufferSize)[3]);

        uPart = 3;

        // update arguments reference
        if(this->m_dgc->GetDWORD32(uOffset+0x1B)!=0x814)
        {
            throw "Unexpected argument reference value.";
        }

        uValue = uBufferSize+0x10;

        // update arguments
        uBOffset = uOffset+0x1B;
        this->SetByte(uBOffset++, ((UCHAR*)&uValue)[0]);
        this->SetByte(uBOffset++, ((UCHAR*)&uValue)[1]);
        this->SetByte(uBOffset++, ((UCHAR*)&uValue)[2]);
        this->SetByte(uBOffset++, ((UCHAR*)&uValue)[3]);

        uPart = 4;

        // update the stack release part, since this function does
        // not have a stack frame (unfortunately)
        if(this->m_dgc->GetDWORD32(uOffset+0xB6)!=0x80C /* cookie */ || this->m_dgc->GetDWORD32(uOffset+0xC5)!=0x808 /* release */)
        {
            throw "Unexpected stack cleanup values.";
        }

        uValue = uBufferSize+8;

        // update cookie variable
        uBOffset = uOffset+0xB6;
        this->SetByte(uBOffset++, ((UCHAR*)&uValue)[0]);
        this->SetByte(uBOffset++, ((UCHAR*)&uValue)[1]);
        this->SetByte(uBOffset++, ((UCHAR*)&uValue)[2]);
        this->SetByte(uBOffset++, ((UCHAR*)&uValue)[3]);

        uValue = uBufferSize+4;

        // change the stack release
        uBOffset = uOffset+0xC5;
        this->SetByte(uBOffset++, ((UCHAR*)&uValue)[0]);
        this->SetByte(uBOffset++, ((UCHAR*)&uValue)[1]);
        this->SetByte(uBOffset++, ((UCHAR*)&uValue)[2]);
        this->SetByte(uBOffset++, ((UCHAR*)&uValue)[3]);
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
