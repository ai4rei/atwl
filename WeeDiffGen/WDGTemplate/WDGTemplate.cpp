#include "WDGTemplate.h"

static /* const */ WDGPLUGININFO l_PluginInfo =
{
    _T("Plugin Name"),
    _T("Plugin Description"),
    _T("Plugin Type/Category"),
    _T("Plugin Group (deprecated)"),
    _T("Author"),
    1, 0,  /* version */
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

LPWDGPLUGININFO WDGPlugin::GetPluginInfo()
{
    return &l_PluginInfo;
}

INT32 WDGPlugin::Enabled()
{
    return 0;
}

INT32 WDGPlugin::Disabled()
{
    return 0;
}

LPCTSTR WDGPlugin::GetInputValue()
{
    return NULL;
}

DiffData* WDGPlugin::GeneratePatch()
{
    return NULL;
}

DiffData* WDGPlugin::GetDiffData()
{
    return NULL;
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
