#ifndef _WDGTEMPLATE_H_
#define _WDGTEMPLATE_H_

#include <tchar.h>

#include <WeeDiffGenPlugin.h>

using namespace WeeDiffGenPlugin;

class WDGPlugin : public IWDGPlugin
{
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
    bool IsSane(void);
};

#endif  /* _WDGTEMPLATE_H_ */
