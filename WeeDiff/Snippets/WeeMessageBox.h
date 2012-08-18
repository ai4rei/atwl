#ifndef _WEE_MESSAGE_BOX_H
#define _WEE_MESSAGE_BOX_H

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

//#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>

#include <CommCtrl.h>
#pragma comment (lib, "comctl32.lib")

#define MB_CHECKBOX			0x10000000

#define MBI_OK				0
#define MBI_WARNING			1
#define MBI_ERROR			2
#define MBI_IMPORTANT		3
#define MBI_QUESTION		4
#define MBI_INFORMATION		5
#define MBI_MAX				6

#define OK_CAPTION			800
#define CANCEL_CAPTION		801
#define ABORT_CAPTION		802
#define RETRY_CAPTION		803
#define IGNORE_CAPTION		804
#define YES_CAPTION			805
#define NO_CAPTION			806
#define CLOSE_CAPTION		807
#define HELP_CAPTION		808
#define TRYAGAIN_CAPTION	809
#define CONTINUE_CAPTION	810

#define IDCHECKBOX			0x00010000

#define IDC_CHECKBOX		1000

typedef struct MSGBOXINFO {
	HINSTANCE hInstance;
	LPCTSTR lpCaption;
	LPCTSTR lpText;
	LPCTSTR lpCheckboxText;
	UINT32 uIconResourceID;
	UINT32 uIconIndex;
	INT32 iStyle;
} MSGBOXINFO, *LPMSGBOXINFO;

INT32 WeeMessageBox(HWND hWnd, LPCTSTR lpszDlgRes, LPMSGBOXINFO mbi);

#endif // _WEE_MESSAGE_BOX_H
