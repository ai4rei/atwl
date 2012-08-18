#include "WDGTemplate.h"

static /* const */ WDGPLUGININFO l_PluginInfo =
{
    _T("Plugin Name"),
    _T("Plugin Description"),
    _T("Plugin Type/Category"),
    _T("Fix|Packet|UI|Data|Add|Color|Auto"),
    _T("Ai4rei/AN"),
    1, 1,  /* targeted patcher version */
    { 0 }  /* guid */
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
    return NULL;
}

DiffData* WDGPlugin::GetDiffData(void)
{
    return NULL;
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

extern "C" __declspec(dllexport) WeeDiffGenPlugin::IWDGPlugin* InitPlugin(LPVOID lpData, USHORT huWeeDiffMajorVersion, USHORT huWeeDiffMinorVersion)
{
    if(l_lpSelfReference)
    {
        DebugBreak();
    }
    else
    {
        l_lpSelfReference = new WDGPlugin(lpData);
    }

    return l_lpSelfReference;
}
