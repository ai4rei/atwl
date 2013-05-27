#ifndef _RAG_EXE_H
#define _RAG_EXE_H

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

#include <Common/WeeDiffGenPlugin.h>

#include "WeeUtility.h"

#include <Windows.h>
#include <tchar.h>
#include <crtdbg.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <string>
#include <exception>

#define BUFP(p,pos) (((unsigned char*)(p)) + (pos))
#define BUFB(p,pos) (*(unsigned char*)((unsigned char*)(p) + (pos)))
#define BUFW(p,pos) (*(unsigned short*)((unsigned char*)(p) + (pos)))
#define BUFL(p,pos) (*(unsigned int*)((unsigned char*)(p) + (pos)))

class RagExe
{
public:
	RagExe(LPCTSTR lpszExePath);
	~RagExe();

	UINT32 Match(WeeDiffGenPlugin::LPFINDDATA lpFindData);
	void Matches(WeeDiffGenPlugin::fnCBAddOffset CBAddOffset, WeeDiffGenPlugin::LPFINDDATA lpFindData);  // TODO: lpContext
	BYTE GetBYTE(UINT32 uOffset) { return *(BYTE *)((UCHAR *)(m_cExeBuffer + uOffset)); }
	WORD GetWORD(UINT32 uOffset) { return *(WORD *)((UCHAR *)(m_cExeBuffer + uOffset)); }
	DWORD32 GetDWORD32(UINT32 uOffset) { return *(DWORD32 *)((UCHAR *)(m_cExeBuffer + uOffset)); }
	INT32 Read(UINT32 uOffset, UCHAR *pBuffer, UINT32 uSize);
	INT32 Replace(WeeDiffGenPlugin::fnCBAddDiffData CBAddDiffData, UINT32 uOffset, WeeDiffGenPlugin::LPFINDDATA lpFindData, bool bZeroTerminate = false);  // TODO: lpContext
	UINT32 FindStr(WeeDiffGenPlugin::LPFINDDATA lpFindData, bool bReturnRva = false);
	PIMAGE_SECTION_HEADER GetSection(CCHAR *lpszSectionName);
	void GetSection(CCHAR *lpszSectionName, PIMAGE_SECTION_HEADER lpImageSectionHeader);
	void GetDOSHeader(PIMAGE_DOS_HEADER lpImageDosHeader);
	void GetNTHeaders(PIMAGE_NT_HEADERS lpImageNTHeaders);
	UINT32 GetNextFreeOffset(UINT32 uSize);
	UINT32 Raw2Rva(UINT32 uOffset);
	UINT32 Rva2Raw(UINT32 uOffset);
	UINT32 FindFunction(CHAR *lpszFunctionName);
	UINT32 GetClientDate() { return m_uClientDate; }

public:
	LPTSTR m_lpszExePath;
	UCHAR *m_cExeBuffer;
	UINT32 m_uExeSize;

protected:
	PIMAGE_DOS_HEADER m_sDosHeader;
	PIMAGE_NT_HEADERS m_sNTHeader;
	std::vector<PIMAGE_SECTION_HEADER> m_vSectionHeaders;		 

	UINT32 m_uClientDate;
	UINT32 m_uLastFreeOffset;
};

#endif // _RAG_EXE_H