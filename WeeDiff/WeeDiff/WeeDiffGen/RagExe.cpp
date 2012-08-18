#include "RagExe.h"

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

#include <time.h>

#define DIFFSECTION_SIZE 1024
#define DIFFSECTION_VSIZE 4096

inline UINT32 AlignTo(UINT32 uSize, UINT32 uBaseSize )
{
	return (((uSize + uBaseSize - 1) / uBaseSize) * uBaseSize);
}

RagExe::RagExe(LPCTSTR lpszExePath)
{
	m_uLastFreeOffset = 0;

	// Check if invalid pointer.
	if(lpszExePath == NULL || lpszExePath[0] == TEXT('\0'))
	{
		throw TEXT("RagExe : Cannot load empty path!");
	}

	// Allocate memory to hold executable path.
	UINT32 iLen = _tcslen(lpszExePath);
	m_lpszExePath = new TCHAR[iLen + 1];

	if(m_lpszExePath == NULL)
	{
		throw TEXT("RagExe : Failed to allocate enough memory for ExePath");
	}

	_tcsncpy_s(m_lpszExePath, iLen + 1, lpszExePath, iLen);

	// Open executable.
	FILE *fp;
	errno_t ferror = _tfopen_s(&fp, lpszExePath, TEXT("rb"));

	if(ferror && fp == NULL)
	{
		delete[] m_lpszExePath;
		throw TEXT("RagExe : Failed to open file!");
	}

	// Get executable information.
	struct _stat stFile = {0};
	_tstat(lpszExePath, &stFile);

	if(stFile.st_size == 0)
	{
		fclose(fp);
		delete[] m_lpszExePath;
		throw TEXT("RagExe : The executable is empty!");
	}

	// Allocate memory to hold the executable.
	m_cExeBuffer = new UCHAR[stFile.st_size];

	if(m_cExeBuffer == NULL)
	{
		fclose(fp);
		delete[] m_lpszExePath;
		throw TEXT("RagExe : Failed to allocate enough memory for executable!");
	}

	m_uExeSize = stFile.st_size;

	if(fread(m_cExeBuffer, 1, m_uExeSize, fp) != m_uExeSize)
	{
		fclose(fp);
		delete[] m_lpszExePath;
		delete[] m_cExeBuffer;
		throw TEXT("RagExe : Failed to read entire executable!");
	}

	fclose(fp);

	m_sDosHeader = (PIMAGE_DOS_HEADER)m_cExeBuffer;

	if(m_sDosHeader->e_magic != IMAGE_DOS_SIGNATURE)
	{
		delete[] m_lpszExePath;
		delete[] m_cExeBuffer;
		throw TEXT("RagExe : Invalid executable!");
	}

	m_sNTHeader = (PIMAGE_NT_HEADERS)((DWORD)(m_sDosHeader) + (m_sDosHeader->e_lfanew));

	if(m_sNTHeader->Signature != IMAGE_NT_SIGNATURE)
	{
		delete[] m_lpszExePath;
		delete[] m_cExeBuffer;
		throw TEXT("RagExe : Invalid PE file!");
	}

	time_t t = (time_t)m_sNTHeader->FileHeader.TimeDateStamp;
	struct tm timeinfo;
	gmtime_s(&timeinfo, &t);

	m_uClientDate = (1900 + timeinfo.tm_year) * 10000 + (timeinfo.tm_mon + 1) * 100 + timeinfo.tm_mday;

	UINT32 uNewExeSize = m_uExeSize + AlignTo(DIFFSECTION_SIZE, m_sNTHeader->OptionalHeader.FileAlignment);
	UCHAR *pNewExeBuffer = new UCHAR[uNewExeSize];
	ZeroMemory(pNewExeBuffer, uNewExeSize);
	memcpy(pNewExeBuffer, m_cExeBuffer, m_uExeSize);
	delete[] m_cExeBuffer;
	m_cExeBuffer = pNewExeBuffer;
	m_uExeSize = uNewExeSize;

	m_sDosHeader = (PIMAGE_DOS_HEADER)m_cExeBuffer;
	m_sNTHeader = (PIMAGE_NT_HEADERS)((DWORD)(m_sDosHeader) + (m_sDosHeader->e_lfanew));

	PIMAGE_SECTION_HEADER sSectionHeader = (PIMAGE_SECTION_HEADER)(m_cExeBuffer + m_sDosHeader->e_lfanew + sizeof(IMAGE_NT_HEADERS32));

	for(USHORT i = 0; i < m_sNTHeader->FileHeader.NumberOfSections; i++)
	{
		m_vSectionHeaders.push_back(sSectionHeader);
		sSectionHeader++;
	}

	UINT32 uSectCount = m_sNTHeader->FileHeader.NumberOfSections;

	sSectionHeader->PointerToRawData = AlignTo(m_vSectionHeaders[uSectCount-1]->PointerToRawData + m_vSectionHeaders[uSectCount-1]->SizeOfRawData, m_sNTHeader->OptionalHeader.FileAlignment);
	sSectionHeader->VirtualAddress = AlignTo(m_vSectionHeaders[uSectCount-1]->VirtualAddress + m_vSectionHeaders[uSectCount-1]->Misc.VirtualSize, m_sNTHeader->OptionalHeader.SectionAlignment);
	sSectionHeader->SizeOfRawData = AlignTo(DIFFSECTION_SIZE, m_sNTHeader->OptionalHeader.FileAlignment);
	sSectionHeader->Misc.VirtualSize = DIFFSECTION_VSIZE;
	sSectionHeader->Characteristics = 0xE0000060;
	memcpy(sSectionHeader->Name, ".diff\x00\x00\x00", 8);

	m_sNTHeader->FileHeader.NumberOfSections++;
	m_sNTHeader->OptionalHeader.SizeOfImage += DIFFSECTION_VSIZE;
	//m_sNTHeader->OptionalHeader.AddressOfEntryPoint = sSectionHeader->VirtualAddress + DIFFSECTION_SIZE;

	m_vSectionHeaders.push_back(sSectionHeader);
}

UINT32 RagExe::Match(WeeDiffGenPlugin::LPFINDDATA lpFindData)
{
	WeeUtility::HEXBUFFER lpPattern = {0};
	PIMAGE_SECTION_HEADER pSection = NULL;
	UINT32 uStart = 0;
	UINT32 uFinish = 0;
	bool bUseWildCard = false;
	CHAR chWildCard = lpFindData->chWildCard;

	if(lpFindData->uMask & WFD_PATTERN)
	{
		lpPattern = WeeUtility::HexStr2Buffer(lpFindData->lpData);
	} else if(lpFindData->uDataSize > 0)
	{
		lpPattern.uSize = lpFindData->uDataSize;
		lpPattern.buffer = (UCHAR *)lpFindData->lpData;
	}

	if(lpFindData->uMask & WFD_SECTION)
	{
		pSection = GetSection(lpFindData->lpszSection);
		if(pSection == NULL)
		{
			if((lpFindData->uMask & WFD_PATTERN) && lpPattern.buffer != NULL)
				delete[] lpPattern.buffer;

			throw "Section not found!";
		}

		uStart = pSection->PointerToRawData;
		uFinish = pSection->PointerToRawData + pSection->SizeOfRawData;
	} else
	{
		uStart = lpFindData->uStart;
		uFinish = lpFindData->uFinish;
	}

	if(lpFindData->uMask & WFD_WILDCARD)
	{
		bUseWildCard = true;
	}

	UINT32 uLength = lpPattern.uSize;

	uStart = (uStart < 0) ? 0 : ((uStart > m_uExeSize - 1) ? m_uExeSize - 1 : uStart);
	uFinish = (uFinish > m_uExeSize || uFinish <= 0) ? m_uExeSize : uFinish;

	if(uLength < 1 || (uStart >= (m_uExeSize - uLength)) || uFinish <= uStart)
	{
		if((lpFindData->uMask & WFD_PATTERN) && lpPattern.buffer != NULL)
			delete[] lpPattern.buffer;

		throw "Invalid arguments provided!";
	}

	UCHAR *pBuffer = m_cExeBuffer + uStart;
	UINT32 uSize = uFinish - uStart;

	UCHAR *p = (UCHAR *)WeeUtility::wildmemmem(pBuffer, uSize, lpPattern.buffer, lpPattern.uSize, bUseWildCard, chWildCard);

	if((lpFindData->uMask & WFD_PATTERN) && lpPattern.buffer != NULL)
		delete[] lpPattern.buffer;

	if(p == NULL)
		throw "Failed to find matching data!";

	return p - m_cExeBuffer;
}

void RagExe::Matches(WeeDiffGenPlugin::fnCBAddOffset CBAddOffset, WeeDiffGenPlugin::LPFINDDATA lpFindData)
{
	UINT32 uCount = 0;
	PIMAGE_SECTION_HEADER pSection = NULL;

	if(lpFindData->uMask & WFD_SECTION)
	{
		pSection = GetSection(lpFindData->lpszSection);
		if(pSection == NULL)
		{
			throw "Section not found!";
		}

		lpFindData->uStart = pSection->PointerToRawData;
		lpFindData->uFinish = pSection->PointerToRawData + pSection->SizeOfRawData;

		lpFindData->uMask ^= WFD_SECTION;
	}

	try
	{
		UINT32 uOldOffset = -1;

		while(true)
		{
			uOldOffset = Match(lpFindData);
			CBAddOffset(uOldOffset);
			lpFindData->uStart = uOldOffset + 1;
			uCount++;
		}
	}
	catch (LPCSTR)
	{
		// Just accept it as it is intended.
	}

	if(uCount <= 0)
		throw "No matching data found!";
}

INT32 RagExe::Read(UINT32 uOffset, UCHAR *pBuffer, UINT32 uSize)
{
	if(uOffset + uSize > m_uExeSize)
		throw "Cannot read behind file size!";

	for(UINT32 i = 0; i < uSize; i++, uOffset++)
	{
		pBuffer[i] = m_cExeBuffer[uOffset];
	}

	return 0;
}

INT32 RagExe::Replace(WeeDiffGenPlugin::fnCBAddDiffData CBAddDiffData, UINT32 uOffset, WeeDiffGenPlugin::LPFINDDATA lpFindData, bool bZeroTerminate)
{
	WeeUtility::HEXBUFFER lpPattern = {0};
	
	if(lpFindData->uMask & WFD_PATTERN)
	{
		lpPattern = WeeUtility::HexStr2Buffer(lpFindData->lpData);
	} else
	{
		lpPattern.buffer = (UCHAR *)lpFindData->lpData;
		lpPattern.uSize = lpFindData->uDataSize;
	}

	if(lpPattern.uSize <= 0)
	{
		return 0;
	}

	if(uOffset + lpPattern.uSize > m_uExeSize)
	{
		delete[] lpPattern.buffer;
		throw "Cannot replace behind file size!";
	}

	for(UINT32 i = 0; i < lpPattern.uSize; i++)
	{
		WeeDiffGenPlugin::DIFFDATA sDiffData = {0};
		sDiffData.iOffset = uOffset++;
		sDiffData.iReplaceValue = lpPattern.buffer[i];

		CBAddDiffData(&sDiffData);
	}

	if(bZeroTerminate == true)
	{
		WeeDiffGenPlugin::DIFFDATA sDiffData = {0};
		sDiffData.iOffset = uOffset;
		sDiffData.iReplaceValue = '\0';

		CBAddDiffData(&sDiffData);
	}

	if(lpFindData->uMask & WFD_PATTERN && lpPattern.buffer != NULL)
	{
		delete[] lpPattern.buffer;
		lpPattern.buffer = NULL;
	}

	return 0;
}

UINT32 RagExe::FindStr(WeeDiffGenPlugin::LPFINDDATA lpFindData, bool bReturnRva /* = false */)
{
	PIMAGE_SECTION_HEADER pSection = GetSection(".rdata");

	if(pSection == NULL)	
		throw "Cannot find .rdata section!";

	WeeUtility::HEXBUFFER sHB = {0};

	if(lpFindData->uMask & WFD_PATTERN)
	{
		sHB = WeeUtility::HexStr2Buffer(lpFindData->lpData);
	} else
	{
		sHB.buffer = (UCHAR *)lpFindData->lpData;
		sHB.uSize = lpFindData->uDataSize;
	}

	WeeDiffGenPlugin::FINDDATA sFindData = {0};

	sFindData.uDataSize = sHB.uSize + 2;
	sFindData.lpData = new CHAR[sHB.uSize + 2];
	sFindData.lpszSection = ".rdata";
	sFindData.uMask = WFD_SECTION;

	ZeroMemory(sFindData.lpData, sFindData.uDataSize);
	memcpy(sFindData.lpData + 1, sHB.buffer, sHB.uSize);

	if(lpFindData->uMask & WFD_PATTERN)
		delete[] sHB.buffer;

	UINT32 uOffset = Match(&sFindData) + 1;

	delete[] sFindData.lpData;

	if(bReturnRva)
	{
		return uOffset + m_sNTHeader->OptionalHeader.ImageBase + (pSection->VirtualAddress - pSection->PointerToRawData);
	}

	return uOffset;
}

PIMAGE_SECTION_HEADER RagExe::GetSection(CCHAR *lpszSectionName)
{
	for(std::vector<PIMAGE_SECTION_HEADER>::iterator it = m_vSectionHeaders.begin(); it != m_vSectionHeaders.end(); it++)
	{
		if(_strnicmp((CCHAR *)(*it)->Name, lpszSectionName, 6) == 0)
		{
			return *it;
		}
	}

	return NULL;
}

void RagExe::GetSection(CCHAR *lpszSectionName, PIMAGE_SECTION_HEADER lpImageSectionHeader)
{
	for(std::vector<PIMAGE_SECTION_HEADER>::iterator it = m_vSectionHeaders.begin(); it != m_vSectionHeaders.end(); it++)
	{
		if(_strnicmp((CCHAR *)(*it)->Name, lpszSectionName, 6) == 0)
		{
			memcpy(lpImageSectionHeader, *it, sizeof(IMAGE_SECTION_HEADER));
			return;
		}
	}

	throw "Section not found!";
}

void RagExe::GetDOSHeader(PIMAGE_DOS_HEADER lpImageDosHeader)
{
	memcpy(lpImageDosHeader, m_sDosHeader, sizeof(IMAGE_DOS_HEADER));
}

void RagExe::GetNTHeaders(PIMAGE_NT_HEADERS lpImageNTHeaders)
{
	memcpy(lpImageNTHeaders, m_sNTHeader, sizeof(IMAGE_NT_HEADERS));
}

UINT32 RagExe::GetNextFreeOffset(UINT32 uSize)
{
	IMAGE_SECTION_HEADER sSection;
	GetSection(".diff", &sSection);

	if(m_uLastFreeOffset < sSection.PointerToRawData)
		m_uLastFreeOffset = sSection.PointerToRawData;

	if(m_uLastFreeOffset + uSize > m_uExeSize)
		throw "There is no more free space in .diff section available!";

	UINT32 uReturnOffset = m_uLastFreeOffset;
	m_uLastFreeOffset += uSize;

	if((m_uLastFreeOffset % 2) != 0)
		m_uLastFreeOffset++;

	return uReturnOffset;
}

UINT32 RagExe::Raw2Rva(UINT32 uOffset)
{
	for(std::vector<PIMAGE_SECTION_HEADER>::iterator it = m_vSectionHeaders.begin(); it != m_vSectionHeaders.end(); it++)
	{
		if(uOffset >= (*it)->PointerToRawData && uOffset < ((*it)->PointerToRawData + (*it)->SizeOfRawData))
		{
			return uOffset + (*it)->VirtualAddress - (*it)->PointerToRawData + m_sNTHeader->OptionalHeader.ImageBase;
		}
	}

	throw "Raw2Rva: No section found that matches the given offset!";
}

UINT32 RagExe::Rva2Raw(UINT32 uOffset)
{
	for(std::vector<PIMAGE_SECTION_HEADER>::iterator it = m_vSectionHeaders.begin(); it != m_vSectionHeaders.end(); it++)
	{
		if(uOffset >= ((*it)->VirtualAddress + m_sNTHeader->OptionalHeader.ImageBase) && uOffset < (((*it)->VirtualAddress + m_sNTHeader->OptionalHeader.ImageBase) + (*it)->Misc.VirtualSize + ((*it)->SizeOfRawData - (*it)->Misc.VirtualSize)))
		{
			return uOffset - m_sNTHeader->OptionalHeader.ImageBase - (*it)->VirtualAddress + (*it)->PointerToRawData;
		}
	}

	throw "Rva2Raw: No section found that matches the given offset!";
}

UINT32 RagExe::FindFunction(CHAR *lpszFunctionName)
{
	IMAGE_SECTION_HEADER sSection;
	GetSection(".rdata", &sSection);

	UINT32 uVirtual = sSection.VirtualAddress - sSection.PointerToRawData;

	UINT32 uLen = strlen(lpszFunctionName);
	UINT32 uOffset = 0;
	CHAR *szName = new CHAR[uLen + 1];
	ZeroMemory(szName, uLen + 1);
	memcpy(szName, lpszFunctionName, uLen);

	WeeDiffGenPlugin::FINDDATA sFindData = {0};
	sFindData.lpData = szName;
	sFindData.uDataSize = uLen + 1;
	sFindData.lpszSection = ".rdata";
	sFindData.uMask = WFD_SECTION;

	uOffset = Match(&sFindData);
	delete[] szName;

	UINT32 uCode = uOffset - 2 + uVirtual;

	ZeroMemory(&sFindData, sizeof(sFindData));
	sFindData.lpData = (CHAR *)&uCode;
	sFindData.uDataSize = 4;
	sFindData.lpszSection = ".rdata";
	sFindData.uMask = WFD_SECTION;

	uOffset = Match(&sFindData);

	return uOffset + uVirtual + m_sNTHeader->OptionalHeader.ImageBase;
}

RagExe::~RagExe()
{
	if(m_lpszExePath != NULL)
	{
		delete[] m_lpszExePath;
		m_lpszExePath = NULL;
	}

	if(m_cExeBuffer != NULL)
	{
		delete[]m_cExeBuffer;
		m_cExeBuffer = NULL;
	}
}