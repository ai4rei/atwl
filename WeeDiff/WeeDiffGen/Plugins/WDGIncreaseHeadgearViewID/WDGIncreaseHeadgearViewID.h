#ifndef _WDG_INCREASE_HEADGEAR_VIEW_ID_H
#define _WDG_INCREASE_HEADGEAR_VIEW_ID_H

#define _CRTDBG_MAP_ALLOC

#include "..\..\..\Common\WeeDiffGenPlugin.h"
#include <tchar.h>
#include <string>

using namespace WeeDiffGenPlugin;

class WDGPlugin : public IWDGPlugin
{
public:
	static const UINT32 MAX_VALUE = 8;

	WDGPlugin(LPVOID lpData) : IWDGPlugin(lpData) 
	{
		ZeroMemory(m_szValue, MAX_VALUE * sizeof(TCHAR));
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
	TCHAR m_szValue[MAX_VALUE];
	DiffData m_diffdata;
};

#endif // _WDG_INCREASE_HEADGEAR_VIEW_ID_H