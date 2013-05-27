#include "WeeUtility.h"

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

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void WeeUtility::LogMsg(UINT32 uClientDate, LPCSTR lpszMsg)
{
	FILE *fp;

	_tfopen_s(&fp, g_szLogPath, TEXT("a"));

	if(fp != NULL)
	{
		CHAR szDate[_MAX_PATH];
		time_t rawtime;

		time(&rawtime);
		struct tm *timeinfo = gmtime(&rawtime);

		strftime(szDate, _MAX_PATH, "%a, %d %b %Y %H:%M:%S GMT", timeinfo);

		fprintf(fp, "%s :: %d :: %s\n", szDate, uClientDate, lpszMsg);

		fclose(fp);
	}
}

WeeUtility::HEXBUFFER WeeUtility::HexStr2Buffer(const CHAR *str)
{
	WeeUtility::HEXBUFFER sHB = {0};
	CHAR chVal[3] = {0};
	USHORT unCurrentPos = 0;

	BYTE bState = 0;
	UINT32 uCount = 0;

	for(UINT32 i = 0; i < strlen(str); i++)
	{
		if(str[i] == '\'')
		{
			bState = (bState == 0);
			continue;
		}

		if(bState == 1)
		{
			if(unCurrentPos > 0)
			{
				unCurrentPos = 0;
				uCount++;
			}

			uCount++;
		}
		else
		{
			if(!isspace(str[i]))
			{
				unCurrentPos++;

				if(unCurrentPos == 2)
				{
					unCurrentPos = 0;
					uCount++;
				}
			}
		}	
	}

	if(uCount == 0)
		return sHB;

	bState = 0;
	unCurrentPos = 0;

	sHB.buffer = new UCHAR[uCount];
	ZeroMemory(sHB.buffer, uCount);

	UINT32 j = 0;
	for(UINT32 i = 0; i < strlen(str); i++)
	{
		if(str[i] == '\'')
		{
			bState = (bState == 0);
			continue;
		}

		if(bState == 1)
		{
			if(unCurrentPos > 0)
			{
				chVal[1] = chVal[0];
				chVal[0] = '0';

				sHB.buffer[j] = (UCHAR)strtol(chVal, NULL, 16);
				unCurrentPos = 0;
				j++;
			}

			sHB.buffer[j] = str[i];
			j++;
		}
		else
		{
			if(!isspace(str[i]))
			{
				chVal[unCurrentPos++] = str[i];

				if(unCurrentPos == 2)
				{
					sHB.buffer[j] = (UCHAR)strtol(chVal, NULL, 16);
					unCurrentPos = 0;
					j++;
				}
			}
		}
	}

	sHB.uSize = uCount;

	return sHB;
}

INT32 WeeUtility::wildmemcmp(LPCVOID lpBuf1, LPCVOID lpBuf2, UINT32 uSize, bool bUseWildCard, CHAR chWildCard)
{
	register const UCHAR *ch1 = (const UCHAR *)lpBuf1;
	register const UCHAR *ch2 = (const UCHAR *)lpBuf2;

	while (uSize-- > 0)
	{
		if(bUseWildCard && *ch2 == (UCHAR)chWildCard)
		{
			ch1++;
			ch2++;
			continue;
		}

		if (*ch1++ != *ch2++)
		{
			return ch1[-1] < ch2[-1] ? -1 : 1;
		}
	}

	return 0;
}

LPVOID WeeUtility::wildmemmem(LPCVOID lpHaystack, UINT32 uHaystackLen, LPCVOID lpNeedle, UINT32 uNeedleLen, bool bUseWildCard, CHAR chWildCard)
{
	CCHAR*chStart;
	CCHAR *const chLastPossible = (CCHAR *)lpHaystack + uHaystackLen - uNeedleLen;

	if(uNeedleLen == 0)
	{
		return (LPVOID)lpHaystack;
	}

	if((uHaystackLen - uNeedleLen) == 0)
		return NULL;

	for(chStart = (CCHAR *)lpHaystack; chStart <= chLastPossible; chStart++)
	{
		if(chStart[0] == ((CCHAR *)lpNeedle)[0] && !WeeUtility::wildmemcmp((LPCVOID)&chStart[1], (LPCVOID)((CCHAR *)lpNeedle + 1), uNeedleLen - 1, bUseWildCard, chWildCard))
		{
			return (LPVOID)chStart;
		}
	}

	return NULL;
}