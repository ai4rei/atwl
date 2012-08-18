#include "Toolbar.h"

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

using namespace WeeTools;

void Toolbar::Init(bool bUseDefault, UINT32 uiResID, HIMAGELIST hImageList, INT32 cx, INT32 cy)
{
	SendMessage(m_hWnd, TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON), 0);

	if(bUseDefault == true) {
		TBADDBITMAP tbAddBmp;

		tbAddBmp.hInst = HINST_COMMCTRL;
		tbAddBmp.nID = uiResID;

		SendMessage(m_hWnd, TB_ADDBITMAP, 0, (LPARAM)&tbAddBmp);
	} else {
		SendMessage(m_hWnd, TB_SETBITMAPSIZE, 0, MAKELONG(cx, cy));
		SendMessage(m_hWnd, TB_SETIMAGELIST, 0, (LPARAM)hImageList);
		SendMessage(m_hWnd, TB_SETBUTTONWIDTH, 0, MAKELONG(50, 50));
	}
}

void Toolbar::AddButton(LPCTSTR lpszStr, INT32 iBitmap, INT32 iCmd, BYTE bStyle)
{
	TBBUTTON tbb = {0};

	tbb.iBitmap = iBitmap;
	tbb.idCommand = iCmd;
	tbb.fsStyle = bStyle;
	tbb.fsState = TBSTATE_ENABLED | TBSTATE_WRAP;
	tbb.iString = (INT_PTR)lpszStr;

	SendMessage(m_hWnd, TB_ADDBUTTONS, 1, (LPARAM)&tbb);
	//SendMessage(m_hWnd, TB_SETBUTTONSIZE, 0, MAKELONG(50, 44));
	//SendMessage(m_hWnd, TB_AUTOSIZE, 0, 0);
}

void Toolbar::EnableButton(const INT32 iCmdId, bool bEnable)
{ 

	if(iCmdId == -1)
	{
		int iButtonCount = SendMessage(m_hWnd, TB_BUTTONCOUNT, 0, 0);
		if(iButtonCount > 0)
		{
			TBBUTTON tbButton;
			for(INT32 i = 0; i < iButtonCount; i++)
			{
				if(SendMessage(m_hWnd, TB_GETBUTTON, i, (LPARAM)&tbButton) == false)
					continue;

				SendMessage(m_hWnd, TB_ENABLEBUTTON, tbButton.idCommand, bEnable);
			}
		}

		return;
	}

	SendMessage(m_hWnd, TB_ENABLEBUTTON, iCmdId, bEnable); 
}