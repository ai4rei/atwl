// -----------------------------------------------------------------
// WDGChatColorGM
// (c) 2012 Ai4rei/AN
//
// This work is licensed under a
// Creative Commons BY-NC-SA 3.0 Unported License
// http://creativecommons.org/licenses/by-nc-sa/3.0/
//
// -----------------------------------------------------------------

#ifndef _WDGCHATCOLORGM_H_
#define _WDGCHATCOLORGM_H_

#include <tchar.h>

#include <WeeDiffGenPlugin.h>

using namespace WeeDiffGenPlugin;

class WDGPlugin : public IWDGPlugin
{
    DiffData m_DiffData;
    COLORREF m_crChatColor;
    TCHAR m_szColor[8];

public:
    WDGPlugin(LPVOID lpData) : IWDGPlugin(lpData)
    {
        this->m_crChatColor = RGB(0xff,0xff,0x00);
        this->m_szColor[0] = 0;
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

#endif  /* _WDGCHATCOLORGM_H_ */
