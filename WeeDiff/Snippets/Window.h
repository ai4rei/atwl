#ifndef _WINDOW_H
#define _WINDOW_H

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

#include <Windows.h>

#include <map>

#include <CommCtrl.h>
#pragma comment (lib, "comctl32.lib")

#include "WeeException.h"
#include "Control.h"

#if defined _M_IX86
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif

namespace WeeTools
{

typedef std::map<int, Control *> ControlsMap;
typedef std::map<int, HFONT> FontMap;

class Window
{
public:
	Window(WNDPROC procMessageHandler, LPCTSTR lpszTitle, INT32 x, INT32 y, INT32 cx, INT32 cy, DWORD32 dwClassStyle, DWORD32 dwStyle, DWORD32 dwExStyle);
	~Window();

	HWND GetHandle() { return m_hWnd; }
	void Show() { ShowWindow(m_hWnd, SW_SHOWDEFAULT); UpdateWindow(m_hWnd); }
	static void Center(HWND hWnd);
	void UseCommonControls(DWORD32 dwFlags);
	static INT32 MessageLoop(HWND hWnd = NULL);
	static POINT GetCenterPoint(HWND hWnd);
	static POINT GetCenterPoint(SIZE s);

	Control *AddControl(Control *c, INT32 iFontID = 0)
	{
		if(c == NULL)
			return NULL;

		m_controls.insert(std::make_pair(c->GetControlID(), c));
		SendMessage(c->GetHandle(), WM_SETFONT, (WPARAM)GetFont(iFontID), TRUE);

		return c;
	}

	Control *GetControl(INT32 iCtrlID) 
	{
		ControlsMap::iterator it = m_controls.find(iCtrlID);

		if(it != m_controls.end())
			return it->second;
		
		return NULL;	
	}

	HFONT AddFont(INT32 iFontID, HFONT hFont)
	{
		m_fonts.insert(std::make_pair(iFontID, hFont));
		return hFont;
	}

	HFONT GetFont(INT32 iFontID)
	{
		FontMap::iterator it = m_fonts.find(iFontID);

		if(it != m_fonts.end())
			return it->second;
		
		return NULL;	
	}

	HDC GetMemDC();

	void SetDefaultFont(HFONT hFont)
	{
		for(ControlsMap::iterator it = m_controls.begin(); it != m_controls.end(); it++)
		{
			SendMessage(it->second->GetHandle(), WM_SETFONT, (WPARAM)hFont, TRUE);
		}
	}

protected:
	HWND m_hWnd;
	WNDCLASSEX m_windowClass;
	ControlsMap m_controls;
	HDC m_hMemDC;
	FontMap m_fonts;
};

}

#endif // _WINDOW_H
