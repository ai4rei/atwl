// -----------------------------------------------------------------
// WDGChatColorParty
// (c) 2012 Ai4rei/AN
//
// This work is licensed under a
// Creative Commons BY-NC-SA 3.0 Unported License
// http://creativecommons.org/licenses/by-nc-sa/3.0/
//
// -----------------------------------------------------------------

#ifndef _WDGCHATCOLORPARTY_H_
#define _WDGCHATCOLORPARTY_H_

#include <tchar.h>

#include <WeeDiffGenPlugin.h>

using namespace WeeDiffGenPlugin;

class WDGPlugin : public IWDGPlugin
{
    enum { SELF = 0, REST };

    DiffData m_DiffData;
    COLORREF m_crChatColor[2];
    TCHAR m_szColor[16];

public:
    WDGPlugin(LPVOID lpData) : IWDGPlugin(lpData)
    {
        this->m_crChatColor[SELF] = RGB(0xff,0xc8,0x00);
        this->m_crChatColor[REST] = RGB(0xff,0xc8,0xc8);
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

#endif  /* _WDGCHATCOLORPARTY_H_ */
