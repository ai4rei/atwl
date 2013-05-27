#ifndef _WEE_UTILITY_H
#define _WEE_UTILITY_H

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

#include "..\Common\WeeDiffGenPlugin.h"
#include <Windows.h>
#include <tchar.h>

#include <string>
#include <algorithm>

extern TCHAR g_szLogPath[];

namespace WeeUtility
{

	typedef struct _HEXBUFFER
	{
		UINT32 uSize;
		UCHAR *buffer;
	}
	HEXBUFFER, *LPHEXBUFFER;

	void LogMsg(UINT32 uClientDate, LPCSTR lpszMsg);
	HEXBUFFER HexStr2Buffer(const CHAR *str);
	INT32 wildmemcmp(LPCVOID lpBuf1, LPCVOID lpBuf2, UINT32 uSize, bool bUseWildCard, CHAR chWildCard);
	LPVOID wildmemmem(LPCVOID lpHaystack, UINT32 uHaystackLen, LPCVOID lpNeedle, UINT32 uNeedleLen, bool bUseWildCard, CHAR chWildCard);
}

#endif // _WEE_UTILITY_H