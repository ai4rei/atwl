#ifndef _WEE_INPUT_BOX_H
#define _WEE_INPUT_BOX_H

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

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <Windows.h>
#include <WindowsX.h>
#include <tchar.h>
#include <CommCtrl.h>
#include <CommDlg.h>
#include <stdlib.h>

#define ID_OK		2000
#define ID_EDIT		2001
#define ID_COLOR	2001

class WeeInputBox
{
	typedef struct _WEEINPUTINFO
	{
		LPTSTR lpszText;
		LPTSTR lpchDst;
		UINT32 uDstSize;
		HFONT hFont;
	}
	WEEINPUTINFO, *LPWEEINPUTINFO;

public:	
	static UINT32 DisplayInputBox(HWND hParent, LPCTSTR lpszCaption, LPCTSTR lpszText, LPTSTR lpchDst, UINT32 uDstSize, HFONT hFont);

private:
	static HGLOBAL CreateTemplate(LPCTSTR lpszCaption);
	static INT_PTR CALLBACK InputProc(HWND hWnd, UINT32 uMsg, WPARAM wParam, LPARAM lParam);
};

#endif