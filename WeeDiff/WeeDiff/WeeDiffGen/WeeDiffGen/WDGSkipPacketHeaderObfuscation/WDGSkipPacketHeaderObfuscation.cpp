// -----------------------------------------------------------------
// WDGSkipPacketHeaderObfuscation
// (c) 2012 Ai4rei/AN
//
// This work is licensed under a
// Creative Commons BY-NC-SA 3.0 Unported License
// http://creativecommons.org/licenses/by-nc-sa/3.0/
//
// -----------------------------------------------------------------

#include "WDGSkipPacketHeaderObfuscation.h"

#ifndef _ARRAYSIZE
    #define _ARRAYSIZE(x) (sizeof(x)/sizeof((x)[0]))
#endif  /* _ARRAYSIZE */

static /* const */ WDGPLUGININFO l_PluginInfo =
{
    _T("Skip Packet Header Obfuscation"),
    _T("Disables incremental packet header obfuscation applied to Zone-Server connections."),
    _T("[Packet]"),
    _T(""),
    _T("Ai4rei/AN"),
    1,
    0,
    { 0x6161b1e5, 0xea19, 0x11e1, { 0xa7, 0x78, 0x0, 0x22, 0x15, 0x64, 0x8a, 0x98 } },  /* guid */
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
    UINT32 uOffset, uPart = 0;

    this->m_DiffData.clear();

    if(!this->IsSane())
    {
        return NULL;
    }

    try
    {
        // MISSION: Make the function, that initializes three
        // obfuscation keys, use always zero disregarding any
        // arguments. There is no reason in hardcoding the constants
        // as they change from time to time.

        // Locate string used as argument to Trace(), which is right
        // after the function we look for.
        Fd.uMask = WFD_PATTERN|WFD_SECTION;
        Fd.lpData = "'PACKET_CZ_ENTER' 00";
        Fd.lpszSection = ".rdata";

        uPart = 1;

        uOffset = this->m_dgc->Match(&Fd);

        // Find the PUSH referencing the string.
        char cPushOffset[5];
        cPushOffset[0] = 0x68;  // PUSH
        ((UINT32*)&cPushOffset[1])[0] = this->m_dgc->Raw2Rva(uOffset);

        Fd.uMask = WFD_SECTION;
        Fd.lpData = cPushOffset;
        Fd.uDataSize = sizeof(cPushOffset);
        Fd.lpszSection = ".text";

        uPart = 2;

        uOffset = this->m_dgc->Match(&Fd);

        // Locate the function by following one of it's CALLs. The
        // call in use is relative-offset one and the offset is
        // added to the offset of the instruction after the call,
        // which is the PUSH from previous part.
        uPart = 3;

        if(this->m_dgc->GetBYTE(uOffset-5)!=0xE8)  // CALL ADDR
        {
            // 'CASH_CATEGORY' insert for non-sakray clients,
            // attempt to find in reach
            Fd.uMask = WFD_PATTERN|WFD_WILDCARD;
            Fd.lpData = "68 '????' 68 '????' 68 '????' E8";  // 3x PUSH DWORD, CALL ADDR
            Fd.chWildCard = '?';
            Fd.uStart = uOffset-0x100;
            Fd.uFinish = uOffset;

            uOffset = this->m_dgc->Match(&Fd)+15+5;
        }
        uOffset+= this->m_dgc->GetDWORD32(uOffset-4);

        // Null.
        uPart = 4;

        if(this->m_dgc->GetBYTE(uOffset)==0x8B)
        {
            this->SetByte(uOffset++,0x33);  // XOR EAX,EAX
            this->SetByte(uOffset++,0xC0);
            this->SetByte(uOffset++,0x33);  // XOR EDX,EDX
            this->SetByte(uOffset++,0xD2);
        }
        else
        {
            throw "Expected MOV R32,[ARG], found something else.";
        }

        // Cherry picking...
        uPart = 5;

        for(BYTE uOp = this->m_dgc->GetBYTE(uOffset); uOp!=0xC2 /* RETN */; uOp = this->m_dgc->GetBYTE(uOffset))
        {
            switch(uOp)
            {
                case 0x8B:  // MOV R32,[ARG]
                    this->SetByte(uOffset,0x89);  // -> MOV [ARG],R32
                    uOffset+= 4;
                    break;
                case 0x89:  // MOV DWORD PTR DS:[R32+x],R32
                    uOffset+= 3;
                    break;
                default:
                    throw "Unexpected op-code.";
            }
        }
    }
    catch(const char* lpszThrown)
    {
        char szErrMsg[1024];

        wsprintfA(szErrMsg, __FILE__" :: Part %u :: %s", uPart, lpszThrown);
        this->m_dgc->LogMsg(szErrMsg);
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
