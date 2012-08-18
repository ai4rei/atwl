#ifndef _WEE_EXCEPTION_H
#define _WEE_EXCEPTION_H

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

#define _CRT_SECURE_NO_WARNINGS

#include <Windows.h>

class WeeException
{
public:
	static const short E_INFO = 0;
	static const short E_WARNING = 1;
	static const short E_ERROR = 2;

	WeeException(LPCTSTR lpszMsg, SHORT nType) { m_lpszMsg = lpszMsg; m_nType = nType; }

	LPCTSTR GetMessage() { return m_lpszMsg; }
	SHORT GetType() { return m_nType; }

protected:
	LPCTSTR m_lpszMsg;
	SHORT m_nType;
};

#endif // _WEE_EXCEPTION_H
