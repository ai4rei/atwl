/************************************************************************/
/*	WeeDiffGen.h
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

#ifndef _WEE_DIFF_GEN_H
#define _WEE_DIFF_GEN_H

#define _CRTDBG_MAP_ALLOC

#include "../Common/WeeDiffPlugin.h"
#include "../Common/WeeDiffGenPlugin.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

extern TCHAR g_szLogPath[];

#include <windows.h>
#include <CommCtrl.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <tchar.h>
#include <crtdbg.h>

#include "WeeInputBox.h"
#include "WeePluginManager.h"
#include "RagExe.h"
#include "WeeDiffCallback.h"
#include "WeeUtility.h"

#ifdef _DEBUG
	#ifndef DBG_NEW
		#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
		#define new DBG_NEW
	#endif // DEBUG_NEW
#endif  // _DEBUG

class WeeDiffGen : public WeePlugin::IPlugin, public WeePlugin::IPlugin2
{
public:

	/************************************************************************/
	/* IPlugin (v1.0)
	/************************************************************************/

	WeeDiffGen(WeePlugin::IGUI *gui) : IPlugin(gui) 
	{
		m_ragexe = NULL;
		m_wpm = NULL;

		m_dgc = new WeeDiffCallback(gui, &m_ragexe);
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

	/************************************************************************/
	/* IPlugin2 (1.1)
	/************************************************************************/

	virtual bool PreventGroupCollision();
	virtual WeePlugin::ColumnNames *GetColumnNames();
	virtual void GetColumnDispInfo(WeePlugin::DIFFITEM *lpDiffItem, NMLVDISPINFO *lpDispInfo);

protected:
	void ClearAll();
	INT32 InitPlugins(LPCTSTR lpszDirectory);
	void ParseDiffs();

protected:
	WeePlugin::DIFFITEMLIST m_diffs;
	RagExe *m_ragexe;
	WeePluginManager *m_wpm;
	WeeDiffCallback *m_dgc;
	TCHAR m_szOutputExe[_MAX_PATH];
	TCHAR m_szDiffTitle[_MAX_PATH];
};


#endif // _WEE_DIFF_GEN_H