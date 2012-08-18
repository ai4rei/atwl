#include "WDGTemplate.h"

#ifndef _ARRAYSIZE
    #define _ARRAYSIZE(x) (sizeof(x)/sizeof((x)[0]))
#endif  /* _ARRAYSIZE */

static /* const */ WDGPLUGININFO l_PluginInfo =
{
    _T("Plugin Name"),
    _T("Plugin Description"),
    _T("[Fix|Packet|UI|Data|Add|Color|Auto]"),
    _T(""),
    _T("Ai4rei/AN"),
    1, 1,  /* targeted patcher version */
    { 0 },  /* guid */
    _T("")
};

static WDGPlugin* l_lpSelfReference = NULL;

void WDGPlugin::Release(void)
{
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
    this->m_DiffData.clear();

    if(!this->IsSane())
    {
        return NULL;
    }

    //

    return this->m_DiffData.empty() ? NULL : &this->m_DiffData;
}

DiffData* WDGPlugin::GetDiffData(void)
{
    return this->m_DiffData.empty() ? NULL : &this->m_DiffData;
}

/**
 * @brief   Makes sure that the image in question is sane, that is,
 *          that it is actually a ragexe of some sort.
 * @return  Returns whether or not the image is sane.
 * @note    The check abuses the fact, that the lowercase word
 *          'gravity' has been in all clients throughout the history
 *          and still remains there, except one catch, those being
 *          encrypted clients that do not qualify 'being sane'.
 ******************************************************************/
bool WDGPlugin::IsSane(void)
{
#define ISSANEMAGIC "gravity"
    FINDDATA Fd;

    Fd.uMask       = WFD_SECTION;
    Fd.lpData      = ISSANEMAGIC;
    Fd.uDataSize   = sizeof(ISSANEMAGIC);
    Fd.lpszSection = ".data";

    try
    {
        this->m_dgc->Match(&Fd);
    }
    catch(const char* lpszThrown)
    {
        return false;

        // unused
        (void)lpszThrown;
    }

    return true;
#undef ISSANEMAGIC
}

void WDGPlugin::SetByte(INT32 nOffset, UCHAR uValue)
{
    DIFFDATA Diff = { nOffset, uValue };

    this->m_DiffData.push_back(Diff);
}

void WDGPlugin::SetWord(INT32 nOffset, USHORT uValue)
{
    this->SetByte(nOffset+0, ((UCHAR*)&uValue)[0]);
    this->SetByte(nOffset+1, ((UCHAR*)&uValue)[1]);
}

void WDGPlugin::SetLong(INT32 nOffset, ULONG uValue)
{
    this->SetWord(nOffset+0, ((USHORT*)&uValue)[0]);
    this->SetWord(nOffset+2, ((USHORT*)&uValue)[1]);
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
