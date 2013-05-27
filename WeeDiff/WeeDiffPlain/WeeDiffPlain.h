#ifndef _WEE_DIFF_PLAIN_H
#define _WEE_DIFF_PLAIN_H

/************************************************************************/
/*	WeeDiffPlain.h
/*	Copyright (C) 2011 Shinryo
/* 
/*  This software is provided 'as-is', without any express or implied
/*  warranty.  In no event will the authors be held liable for any damages
/*  arising from the use of this software.
/* 
/*	You are allowed to alter this software and redistribute it freely with
/*	the following restrictions:
/*	
/*	1. You must not claim that you wrote the original software.
/*	2. Do not represent your altered source version as original software.
/*	3. This notice may not be removed or altered from any source distribution.
/*	
/************************************************************************/

#include <Common/WeeDiffPlugin.h>

#include <vector>
#include <map>

typedef struct _PLAINDIFFITEM
{
	INT32 iOffset;
	// Even though they use only 1 byte, they are defined here as 4 bytes in case this plug-in should change.
	// Which I doubt will ever happen..
	INT32 iOriginalValue;
	INT32 iReplaceValue;
}
PLAINDIFFITEM, *LPPLAINDIFFITEM;

typedef struct _AUTOSAVE
{
	INT32 iHash;
	LPTSTR lpszString;
}
AUTOSAVE, *LPAUTOSAVE;


class WeeDiffPlain : public WeePlugin::IPlugin
{
public:
	typedef std::vector<PLAINDIFFITEM> DiffItemContainer;

	WeeDiffPlain(WeePlugin::IGUI *gui) : IPlugin(gui)  
	{
		m_iOriginalCRC = 0;
		ZeroMemory(&m_lpszBlurb, sizeof(m_lpszBlurb));
	}

	/************************************************************************/
	/* See WeeDiffPlugin.h
	/************************************************************************/
	virtual void Release();
	virtual void Free(LPVOID memory);
	virtual void About(HWND hParent);
	virtual WeePlugin::LPWEEPLUGININFO GetPluginInfo();
	virtual LPCTSTR GetSupportedFormat();
	virtual LPCTSTR GetDiffTitle();
	virtual void Notify(WeePlugin::LPNOTIFYMESSAGE sMessage);
	virtual bool NeedDiffFile();
	virtual INT32 PatchIt();
	virtual INT32 GetDiffCount();
	virtual WeePlugin::LPDIFFITEMLIST GetDiffItems();

protected:
	void ClearAll(); // 
	INT32 ParseDiffFile(LPCTSTR lpszDiffPath);

	// Used to find the container of diff items that already exist.
	DiffItemContainer *FindDiffContainer(WeePlugin::DIFFITEM *lpsDiffItem);

protected:
	TCHAR m_lpszBlurb[256]; // For the time being, assume that no diff title in this world is this long.
	UINT32 m_iOriginalCRC;
	WeePlugin::DIFFITEMLIST m_diffs;
};

#endif // _WEE_DIFF_PLAIN_H
