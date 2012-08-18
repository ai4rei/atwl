#ifndef _CONTROL_H
#define _CONTROL_H

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

#include <CommCtrl.h>
#pragma comment (lib, "comctl32.lib")

typedef struct INITCONTROL
{
	HINSTANCE hInstance;
	HWND hParent;
	LPCTSTR lpszClassName;
	DWORD32 dwExStyle;
	DWORD32 dwStyle;
	INT32 x;
	INT32 y;
	INT32 cx;
	INT32 cy;
	INT32 id;
	LPCTSTR lpszCaption;
	WNDPROC procDoubleBuffer;
} INITCONTROL, *LPINITCONTROL;

namespace WeeTools
{

class Control
{
public:
	Control(LPINITCONTROL lpInitControl);
	HWND GetHandle() const { return m_hWnd; }
	HWND GetParent() const { return m_hParent; }
	INT32 GetControlID() const { return m_iCtrlID; }
	WNDPROC GetOriginalWndProc() const { return m_procOriginal; }

protected:
	INT32 m_iCtrlID;
	HWND m_hWnd;
	HWND m_hParent;
	WNDPROC m_procDoubleBuffer;
	WNDPROC m_procOriginal;
};

}

#endif // _CONTROL_H
