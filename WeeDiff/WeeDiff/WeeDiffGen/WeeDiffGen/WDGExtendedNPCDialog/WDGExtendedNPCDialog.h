// -----------------------------------------------------------------
// WDGExtendedNPCDialog
// (c) 2012 Ai4rei/AN
//
// This work is licensed under a
// Creative Commons BY-NC-SA 3.0 Unported License
// http://creativecommons.org/licenses/by-nc-sa/3.0/
//
// -----------------------------------------------------------------

#ifndef _WDGEXTENDEDNPCDIALOG_H_
#define _WDGEXTENDEDNPCDIALOG_H_

#include <tchar.h>

#include <WeeDiffGenPlugin.h>

using namespace WeeDiffGenPlugin;

class WDGPlugin : public IWDGPlugin
{
    DiffData m_DiffData;
    TCHAR m_szBufferSize[16];

public:
    WDGPlugin(LPVOID lpData) : IWDGPlugin(lpData)
    {
        this->m_szBufferSize[0] = 0;
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
};

#endif  /* _WDGEXTENDEDNPCDIALOG_H_ */
