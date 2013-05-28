#ifndef _WDG_CUSTOM_WINDOW_TITLE_H
#define _WDG_CUSTOM_WINDOW_TITLE_H

#define _CRTDBG_MAP_ALLOC

#include <WeeDiffGenPlugin.h>
#include <tchar.h>
#include <string>

using namespace WeeDiffGenPlugin;

class WDGPlugin : public IWDGPlugin
{
public:
	static const UINT32 MAX_TITLE = 70;

	WDGPlugin(LPVOID lpData) : IWDGPlugin(lpData) 
	{
		ZeroMemory(m_szValue, MAX_TITLE * sizeof(TCHAR));
	}

	virtual void Release();
	virtual void Free(LPVOID memory);
	virtual LPWDGPLUGININFO GetPluginInfo();
	virtual INT32 Enabled();
	virtual INT32 Disabled();
	virtual LPCTSTR GetInputValue();
	virtual DiffData *GeneratePatch();
	virtual DiffData *GetDiffData();

protected:
	static void __stdcall CBAddDiffData(WeeDiffGenPlugin::LPDIFFDATA lpDiffData);

protected:
	TCHAR m_szValue[MAX_TITLE];
	DiffData m_diffdata;
};

#endif // _WDG_CUSTOM_WINDOW_TITLE_H