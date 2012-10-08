// -----------------------------------------------------------------
// WDGTranslateClient
// (c) 2012 Ai4rei/AN
//
// This work is licensed under a
// Creative Commons BY-NC-SA 3.0 Unported License
// http://creativecommons.org/licenses/by-nc-sa/3.0/
//
// -----------------------------------------------------------------

#ifndef _WDGTRANSLATECLIENT_H_
#define _WDGTRANSLATECLIENT_H_

#include <tchar.h>

#include <WeeDiffGenPlugin.h>

// <!HACK[[
TCHAR g_szLogPath[MAX_PATH];
#include "../../WeeUtility.h"
#include "../../WeeUtility.cpp"
// ]]>

using namespace WeeDiffGenPlugin;

#include "XLateBE.h"  // backend code

class WDGPlugin : public IWDGPlugin
{
    DiffData m_DiffData;

public:
    WDGPlugin(LPVOID lpData) : IWDGPlugin(lpData){}
    //
    virtual void Release(void);
    virtual void Free(LPVOID lpBuf);
    virtual LPWDGPLUGININFO GetPluginInfo(void);
    virtual INT32 Enabled(void);
    virtual INT32 Disabled(void);
    virtual LPCTSTR GetInputValue(void);
    virtual DiffData* GeneratePatch(void);
    virtual DiffData* GetDiffData(void);

protected:
    static bool CALLBACK ApplyTranslation(TLITEM* lpItem, void* lpContext);

private:
    bool IsVC9Image(void);
    bool IsSane(void);
    void SetByte(UINT32 uOffset, UCHAR uValue);
    void SetBuffer(UINT uOffset, CHAR* lpBuffer, UINT32 uSize);
    bool TryMatch(LPFINDDATA lpFd, UINT32* lpuOffset);
};

#endif  /* _WDGTRANSLATECLIENT_H_ */
