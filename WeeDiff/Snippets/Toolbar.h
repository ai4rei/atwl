#ifndef _TOOLBAR_H
#define _TOOLBAR_H

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
#include "Control.h"

namespace WeeTools
{

class Toolbar : public Control
{
public:
	Toolbar(LPINITCONTROL lpInitControl) : Control(lpInitControl)
	{
	}

	void Init(bool bUseDefault, UINT32 uiResID, HIMAGELIST hImageList, INT32 cx, INT32 cy);
	void AddButton(LPCTSTR lpszStr, INT32 iBitmap, INT32 iCmd, BYTE bStyle);
	void EnableButton(const INT32 iCmdId, bool bEnable);

	void DelButton(INT32 iCmdId)
	{
		int iIndex = SendMessage(m_hWnd, TB_COMMANDTOINDEX, iCmdId, 0);
		if(iIndex != -1)
			SendMessage(m_hWnd, TB_DELETEBUTTON, iIndex, 0);
	}

};

}

#endif // _TOOLBAR_H