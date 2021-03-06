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
    virtual void Release(void);
    virtual void Free(LPVOID lpBuf);
    virtual LPWDGPLUGININFO GetPluginInfo(void);
    virtual INT32 Enabled(void);
    virtual INT32 Disabled(void);
    virtual LPCTSTR GetInputValue(void);
    virtual DiffData* GeneratePatch(void);
    virtual DiffData* GetDiffData(void);

private:
    void SetByte(UINT32 uOffset, UCHAR uValue);
    void WDGPlugin::SetLong(UINT32 uOffset, ULONG uValue);
};

#endif  /* WDGALLOWUNKNOWNSLASHCOMMANDS_H */
