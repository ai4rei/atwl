// -----------------------------------------------------------------
// WDGDisableHShield
// (c) 2013 Ai4rei/AN
//
// This work is licensed under a
// Creative Commons BY-NC-SA 3.0 Unported License
// http://creativecommons.org/licenses/by-nc-sa/3.0/
//
// -----------------------------------------------------------------

#include "WDGDisableHShield.h"

#ifndef _ARRAYSIZE
    #define _ARRAYSIZE(x) (sizeof(x)/sizeof((x)[0]))
#endif  /* _ARRAYSIZE */

static /* const */ WDGPLUGININFO l_PluginInfo =
{
    _T("Disable HShield"),
    _T("Prevents AhnLabs HackShield from beeing loaded during client start up."),
    _T("[Fix]"),
    _T(""),
    _T("Ai4rei/AN"),
    1,
    0,
    { 0x29367b42, 0xb3c4, 0x494c, { 0x9a, 0xb1, 0x54, 0x52, 0x0f, 0xaa, 0x0c, 0xc1 } },  /* guid */
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
    bool bSuccess = false;
    FINDDATA Fd;
    UINT32 uOffset, uBOffset, uPart = 0;

    this->m_DiffData.clear();

    if(!this->IsSane())
    {
        return NULL;
    }

    try
    {
        // MISSION: Neuter HackShield-related functions. If we could
        // use 'find referenced call', then things would be easy...

        // find server string
        Fd.uMask = WFD_PATTERN|WFD_SECTION;
        Fd.lpData = "'webclinic.ahnlab.com'";
        Fd.lpszSection = this->IsVC9Image() ? ".rdata" : ".data";

        uPart = 1;
        uOffset = this->m_dgc->Match(&Fd);

        // PUSH LONG CONST
        char cMatchStr1[5] = { 0x68 };
        ((UINT32*)&cMatchStr1[1])[0] = this->m_dgc->Raw2Rva(uOffset);
    
        // find reference
        Fd.uMask = WFD_SECTION;
        Fd.lpData = cMatchStr1;
        Fd.uDataSize = sizeof(cMatchStr1);
        Fd.lpszSection = ".text";

        uPart = 2;
        uOffset = this->m_dgc->Match(&Fd);

        // trace back to nearest JE pointing at us,
        // followed by a XOR EAX,EAX
        uPart = 3;
        for(uBOffset = uOffset-4; uOffset-uBOffset<0x80+2; uBOffset--)
        {
            bSuccess = this->m_dgc->GetBYTE(uBOffset+0)==0x74                   // JE ...
                    && this->m_dgc->GetBYTE(uBOffset+1)==uOffset-uBOffset-2     // ... SHORT OFFSET v
                    && this->m_dgc->GetBYTE(uBOffset+2)==0x33                   // XOR ...
                    && this->m_dgc->GetBYTE(uBOffset+3)==0xC0                   // ... EAX,EAX
                    ;

            if(bSuccess)
            {
                break;
            }
        }
        if(!bSuccess)
        {// not found
            throw "Function signature changed.";
        }
        uOffset = uBOffset;

        // replace with a success value
        this->SetByte(uOffset++,0x33);  // XOR     EAX,EAX
        this->SetByte(uOffset++,0xC0);
        this->SetByte(uOffset++,0x40);  // INC     EAX
        this->SetByte(uOffset++,0x90);  // NOP

        // go for double-layered setup (VC9 only)
        if(this->IsVC9Image())
        {
            // find an error
            Fd.uMask = WFD_PATTERN|WFD_SECTION;
            Fd.lpData = "00 'ERROR'";  // assumption to prevent NIGHTMARE_TERROR from getting cought
            Fd.lpszSection = ".rdata";

            uPart = 4;
            uOffset = this->m_dgc->Match(&Fd)+1;

            // PUSH LONG CONST; PUSH EAX
            char cMatchStr2[6] = { 0x68, 0x00, 0x00, 0x00, 0x00, 0x50 };
            ((UINT32*)&cMatchStr2[1])[0] = this->m_dgc->Raw2Rva(uOffset);

            // find reference
            Fd.uMask = WFD_SECTION;
            Fd.lpData = cMatchStr2;
            Fd.uDataSize = sizeof(cMatchStr2);
            Fd.lpszSection = ".text";

            uPart = 5;
            uOffset = this->m_dgc->Match(&Fd)+sizeof(cMatchStr2);

            // trace down for a CMP BYTE PTR DS:[CONST],0; JNZ
            uPart = 6;
            for(uBOffset = uOffset; uBOffset-uOffset<0x80; uBOffset++)
            {
                bSuccess = this->m_dgc->GetBYTE(uBOffset+0)==0x80
                        && this->m_dgc->GetBYTE(uBOffset+1)==0x3D
                        && this->m_dgc->GetBYTE(uBOffset+6)==0x00
                        && this->m_dgc->GetBYTE(uBOffset+7)==0x75
                        ;

                if(bSuccess)
                {
                    break;
                }
            }
            if(!bSuccess)
            {// not found
                throw "Function signature changed.";
            }

            // make it always jump
            this->SetByte(uBOffset+7,0xEB);
        }
    }
    catch(const char* lpszThrown)
    {
        char szErrMsg[1024];

        wsprintfA(szErrMsg, __FILE__" :: Part %u :: %s", uPart, lpszThrown);
        this->m_dgc->LogMsg(szErrMsg);
        this->m_DiffData.clear();  // clear partial diff
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
