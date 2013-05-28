#ifndef _WDG_ENFORCE_OFFICIA_LOGIN_BACKGROUND_H
#define _WDG_ENFORCE_OFFICIA_LOGIN_BACKGROUND_H

#define _CRTDBG_MAP_ALLOC

#include <WeeDiffGenPlugin.h>
#include <tchar.h>
#include <string>

using namespace WeeDiffGenPlugin;

class WDGPlugin : public IWDGPlugin
{
public:
	WDGPlugin(LPVOID lpData) : IWDGPlugin(lpData) {}

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
	static void __stdcall CBAddOffset(UINT32 uOffset);

protected:
	DiffData m_diffdata;
	std::vector<UINT32> *m_offsets;
};

#endif // _WDG_ENFORCE_OFFICIA_LOGIN_BACKGROUND_H