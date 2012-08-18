#ifndef _WEE_GUI_INTERFACE_H
#define _WEE_GUI_INTERFACE_H

/************************************************************************/
/*	WeeDiffPlain.c
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

#include "../Common/WeeDiffPlugin.h"
#include "../../Snippets/WeeMessageBox.h"

#include "resource.h"
#include "weeresource.h"

class WeeGUI : WeePlugin::IGUI
{
public:
	virtual INT32 DisplayMessageBox(LPCTSTR lpszCaption, LPCTSTR lpszText, LPCTSTR lpszCheckbox, UINT32 uIcondIndex, INT32 iStyle)
	{
		MSGBOXINFO mbi;

		mbi.hInstance = m_hInstance;
		mbi.lpCaption = lpszCaption;
		mbi.lpText = lpszText;
		mbi.lpCheckboxText = lpszCheckbox;
		mbi.uIconIndex = uIcondIndex;
		mbi.iStyle = iStyle;
		mbi.uIconResourceID = IDB_WMB;

		return WeeMessageBox(m_hParent, MAKEINTRESOURCE(IDD_WMB), &mbi);
	}

	virtual HWND GetMainHandle()
	{
		return m_hParent;
	}

	WeeGUI(HINSTANCE hInstance, HWND hParent)
	{
		m_hInstance = hInstance;
		m_hParent = hParent;
	}

private:
	HINSTANCE m_hInstance;
	HWND m_hParent;
};

#endif // _WEE_GUI_INTERFACE_H