// -----------------------------------------------------------------
// WDGAllowUnknownSlashCommands
// (c) 2021 Ai4rei/AN
//
// This work is licensed under a
// Creative Commons BY-NC-SA 4.0 International License
// https://creativecommons.org/licenses/by-nc-sa/4.0/
//
// -----------------------------------------------------------------

#ifndef WDGALLOWUNKNOWNSLASHCOMMANDS_H
#define WDGALLOWUNKNOWNSLASHCOMMANDS_H

#include <tchar.h>

#include <WeeDiffGenPlugin.h>

using namespace WeeDiffGenPlugin;

class WDGPlugin : public IWDGPlugin
{
    DiffData m_DiffData;

public:
    WDGPlugin(LPVOID lpData) : IWDGPlugin(lpData)
    {
    }
    //
    virtual void Release();
    virtual void Free(LPVOID lpBuf);
    virtual LPWDGPLUGININFO GetPluginInfo();
    virtual INT32 Enabled();
    virtual INT32 Disabled();
    virtual LPCTSTR GetInputValue();
    virtual DiffData* GeneratePatch();
    virtual DiffData* GetDiffData();

private:
    DiffData* GeneratePatchV1(UINT32 uOffset);
    void SetByte(UINT32 uOffset, UCHAR uValue);
};

#endif  /* WDGALLOWUNKNOWNSLASHCOMMANDS_H */
