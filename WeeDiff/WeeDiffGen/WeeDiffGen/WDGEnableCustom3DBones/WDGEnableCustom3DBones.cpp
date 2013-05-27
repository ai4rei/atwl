// -----------------------------------------------------------------
// WDGEnableCustom3DBones
// (c) 2013 Ai4rei/AN, curiosity, Animated 3D Monsters in RO Project
//
// This work is licensed under a
// Creative Commons BY-NC-SA 3.0 Unported License
// http://creativecommons.org/licenses/by-nc-sa/3.0/
//
// -----------------------------------------------------------------

#include "WDGEnableCustom3DBones.h"

#ifndef _ARRAYSIZE
    #define _ARRAYSIZE(x) (sizeof(x)/sizeof((x)[0]))
#endif  /* _ARRAYSIZE */

static /* const */ WDGPLUGININFO l_PluginInfo =
{
    _T("Enable Custom 3D Bones"),
    _T("Enables the use of custom 3D monsters (Granny) by lifting hardcoded ID limit."),
    _T("[Data]"),
    _T(""),
    _T("Ai4rei/AN"),
    1,
    0,
    { 0x6161b1f6, 0xea19, 0x11e1, { 0xa7, 0x78, 0x0, 0x22, 0x15, 0x64, 0x8a, 0x98 } },  /* guid */
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
    UINT32 uOffset, uBOffset, uPart = 0;

    this->m_DiffData.clear();

    if(!this->IsSane())
    {
        return NULL;
    }

    try
    {
        // MISSION: Disable hard-coded ID limit of 9 and bypass
        // Type2MobId table for IDs 10+.

        // Find C3dGrannyBoneRes::GetAnimation by bone (Part 1).
        Fd.uMask = WFD_PATTERN|WFD_SECTION;
        Fd.lpData = "'model\\3dmob_bone\\%d_%s.gr2' 00";
        Fd.lpszSection = this->IsVC9Image() ? ".rdata" : ".data";
        uPart = 1;

        uOffset = this->m_dgc->Match(&Fd);

        // Find C3dGrannyBoneRes::GetAnimation by bone (Part 2).
        char cPushRef[5] = { 0x68 };
        Fd.uMask = WFD_SECTION;
        Fd.lpData = cPushRef;
        Fd.uDataSize = sizeof(cPushRef);
        Fd.lpszSection = ".text";
        ((UINT32*)&cPushRef[1])[0] = this->m_dgc->Raw2Rva(uOffset);
        uPart = 2;

        uBOffset = this->m_dgc->Match(&Fd)-2-7;  // 2x PUSH R32, 1x MOV R32,[<ARRAY>]

        // Find limiting CMP
        try
        {
            Fd.uMask = WFD_PATTERN;
            Fd.lpData = "83FE 09";   // CMP     ESI,9h
            Fd.uStart = uBOffset-0x70;  // approximate function start
            Fd.uFinish = uBOffset;
            uPart = 3;

            uOffset = this->m_dgc->Match(&Fd)+3;  // VC9
        }
        catch(const char*)
        {
            Fd.uMask = WFD_PATTERN;
            Fd.lpData = "83FE 0A";   // CMP     ESI,10h
            Fd.uStart = uBOffset-0x70;  // approximate function start
            Fd.uFinish = uBOffset;
            uPart = 4;

            uOffset = this->m_dgc->Match(&Fd)+3;  // VC6
        }

        // Modify JGE/JA to always address bones. Do not care about
        // which CMP we hit, the important thing is the conditional
        // JGE/JA after it, be it SHORT or LONG.
        // Also let's trust the client here, that it never calls the
        // function with nAniIdx outside of [0;4].
        switch(this->m_dgc->GetBYTE(uOffset))
        {
            case 0x77:
            case 0x7d:
                // SHORT
                this->SetByte(uOffset+1, (UCHAR)(uBOffset-(uOffset+2)));
                break;
            case 0x0f:
                // LONG
                this->SetByte(uOffset+2, (UCHAR)(uBOffset-(uOffset+6)));
                break;
            default:
                // OOPS
                throw /* up */ "Hit something unexpected; something that does not look like a JGE/JA.";
        }
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
