// -----------------------------------------------------------------
// WDGCancelToLoginWindow
// (c) 2013-2014 Neo
//
// <no license information>
//
// -----------------------------------------------------------------

#ifndef _WDGCANCELTOLOGINWINDOW_H_
#define _WDGCANCELTOLOGINWINDOW_H_

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
    bool IsVC9Image(void);
    bool IsSane(void);
    void SetByte(UINT32 uOffset, UCHAR uValue);
    void SetBuffer(UINT uOffset, CHAR* lpBuffer, UINT32 uSize);
    bool TryMatch(LPFINDDATA lpFd, UINT32* lpuOffset);
};

#endif  /* _WDGCANCELTOLOGINWINDOW_H_ */
