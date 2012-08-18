#include "WeeDiffCallback.h"

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

#pragma warning(disable: 4996)

void WeeDiffCallback::LogMsg(LPCSTR lpszMsg)
{
	WeeUtility::LogMsg((*m_ragexe)->GetClientDate(), lpszMsg);
}

WeeDiffCallback::WeeDiffCallback(WeePlugin::IGUI *gui, RagExe **re)
{
	m_gui = gui;
	m_ragexe = re;

	OSVERSIONINFO osvi;
	ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&osvi);

	NONCLIENTMETRICS ncm;
	ncm.cbSize = sizeof(NONCLIENTMETRICS) - (osvi.dwMajorVersion >= 6 ? 0 : 4);

	SystemParametersInfo(SPI_GETNONCLIENTMETRICS, ncm.cbSize, &ncm, 0);

	m_hFont = CreateFontIndirect(&ncm.lfMessageFont);
}
