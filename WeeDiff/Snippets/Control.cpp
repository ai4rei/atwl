#include "Control.h"
#include "WeeException.h"

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

Control::Control(LPINITCONTROL lpInitControl)
{
	HWND m_hParent = lpInitControl->hParent;
	WNDPROC m_procDoubleBuffer = lpInitControl->procDoubleBuffer;
	m_iCtrlID = lpInitControl->id;

	m_hWnd = CreateWindowEx(
				lpInitControl->dwExStyle, 
				lpInitControl->lpszClassName, 
				lpInitControl->lpszCaption, 
				WS_CHILD | WS_VISIBLE | lpInitControl->dwStyle, 
				lpInitControl->x, 
				lpInitControl->y, 
				lpInitControl->cx, 
				lpInitControl->cy, 
				m_hParent, 
				(HMENU)lpInitControl->id, 
				lpInitControl->hInstance, 
				NULL);

	if(m_hWnd == NULL)
		throw new WeeException(TEXT(__FUNCTION__) TEXT(": Failed to create control!"), WeeException::E_ERROR);

	if(m_procDoubleBuffer != NULL)
		m_procOriginal = (WNDPROC)SetWindowLongPtr(m_hWnd, GWLP_WNDPROC, (LONG_PTR)m_procDoubleBuffer);
}
