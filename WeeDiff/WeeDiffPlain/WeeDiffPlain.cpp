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

// Debugging purposes.

#include <stdlib.h>
#include <crtdbg.h>

#ifdef _DEBUG
	#ifndef DBG_NEW
		#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
		#define new DBG_NEW
	#endif // DEBUG_NEW
#endif  // _DEBUG

// Main content starts here.

#include "WeeDiffPlain.h"

// Include some basic stuff.
#include <windows.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <tchar.h>

// Reference to itself.
WeeDiffPlain *g_WeeDiffPlain = NULL;

// Saved paths to be used later on.
TCHAR g_szDllPath[_MAX_PATH] = {0};
TCHAR g_szInputExePath[_MAX_PATH] = {0};
TCHAR g_szInputDiffPath[_MAX_PATH] = {0};
TCHAR g_szOutputExePath[_MAX_PATH] = {0};

// Default check box text.
LPCTSTR g_lpszCheckboxMsg = TEXT("Don't show this dialog again for this session");

//////////////////////////////////////////////////////////////////////////
// Random snippets

// 0-8-15 string id generator.
// Note: DiffPatcher itself always checks szName if it finds
// a hash generated by this or any other function.
INT32 GenerateHash(const TCHAR *szName)
{
	INT32 iHash = 0;

	while(*szName)
	{
		iHash = (iHash << 4) + (iHash >> 5) * 12 + *szName;
		szName++;
	}

	return iHash & 0xFFFF;
}

// Simple implementation if case-insensitive strstr(..)
TCHAR *stristr(LPCTSTR _str, LPCTSTR _substr)
{
	LPCTSTR p;

	for(p = _str; *p != NULL; p++)
	{
		while(*p != NULL && toupper(*p) != toupper(*_substr))
			p++;

		if(*p == NULL)
			return NULL;

		TCHAR *pStr = (TCHAR *)p;
		TCHAR *pSubStr = (TCHAR *)_substr;

		while(toupper(*pStr) == toupper(*pSubStr))
		{
			pStr++;
			pSubStr++;

			if(*pSubStr == NULL)
				return (TCHAR *)pSubStr;
		}
	}

	return NULL;
}

// Simple CRC calculation.
ULONG32 uCRCTable[256];
ULONG32 uCRC32Polynomial = 0xEDB88320L;

void CRCGenerateTable()
{
	// Use registers to speed up generation.
	register INT32 i, j;
	register ULONG32 uCRC;

	for(i = 0; i < 256; i++)
	{
		uCRC = i;
		for(j = 8; j > 0; j--)
		{
			if(uCRC & 1)
				uCRC = (uCRC >> 1) ^ uCRC32Polynomial;
			else
				uCRC >>= 1;
		}

		uCRCTable[i] = uCRC;
	}
}

ULONG32 CRCCalcBuffer(UCHAR *buffer, ULONG32 uSize)
{
	ULONG32 uCRC = 0xFFFFFFFF;

	register ULONG32 i;

	for(i = 0; i < uSize; i++)
	{
		uCRC = (uCRC >> 8) ^ uCRCTable[(uCRC ^ buffer[i]) & 0xFF];
	}

	// Don't ask me why all diffs have cut down their crc to 8 numbers. :(
	return (uCRC ^ 0xFFFFFFFF);
}

// Thanks to Ai4rei for the information regarding Maldiablo's way to handle CRC values higher than 0x7FFFFFFF. 
ULONG32 Crc32ToOCrc(ULONG32 uCrc32)
{
	if(uCrc32 > 0x7FFFFFFF)
	{
		uCrc32 = (~uCrc32) + 1;
	}

	return uCrc32 % 100000000;
}

// Simple trim implementation.
TCHAR *TrimSpaces(TCHAR *str)
{
	TCHAR *end;

	while(isspace(*str)) 
		str++;

	if(*str == 0)
		return str;

	end = str + _tcslen(str) - 1;
	while(end > str && isspace(*end)) 
		end--;

	*(end+1) = TEXT('\0');

	return str;
}

//////////////////////////////////////////////////////////////////////////
// Class functions

void WeeDiffPlain::Release()
{
	ClearAll();
	g_WeeDiffPlain = NULL;
	delete this;
}

void WeeDiffPlain::Free(LPVOID memory)
{
	delete memory;
}

void WeeDiffPlain::About(HWND hParent)
{
	// Note: You can create here whatever box you like.
	m_gui->DisplayMessageBox(GetPluginInfo()->lpszPluginName, TEXT("This is just a simple implementation of the plain txt based diff (*.diff) format."), NULL, MBI_INFORMATION, MB_OK);
}

WeePlugin::LPWEEPLUGININFO WeeDiffPlain::GetPluginInfo()
{
	static WeePlugin::WEEPLUGININFO wpi = {0};

	if(((TCHAR *)&wpi)[0] == 0)
	{
		// Unique ID
		static GUID g = { 0xf401750f, 0x3291, 0x488e, { 0xbc, 0x1c, 0x9f, 0x4e, 0x36, 0x8b, 0x9b, 0x2e } };

		wpi.lpszAuthorName = TEXT("Shinryo");
		wpi.lpszPluginName = TEXT("Plain Diff Plugin v1.0");
		wpi.unMinMajorVersion = 1;
		wpi.unMinMinorVersion = 0;
		wpi.guid = g;
	}

	return &wpi;
}

LPCTSTR WeeDiffPlain::GetSupportedFormat()
{
	return TEXT(".diff");
}

LPCTSTR WeeDiffPlain::GetDiffTitle()
{
	if(_tcslen(m_lpszBlurb) <= 0)
		return NULL;

	return m_lpszBlurb;
}

void WeeDiffPlain::Notify(WeePlugin::LPNOTIFYMESSAGE sMessage)
{
	switch(sMessage->iMessage)
	{
	case WeePlugin::NM_EXE_CHANGED:		
		_tcsncpy_s(g_szInputExePath, (const TCHAR *)sMessage->lpData, _MAX_PATH);
		break;
	case WeePlugin::NM_DIFF_CHANGED:
		{
			_tcsncpy_s(g_szInputDiffPath, (const TCHAR *)sMessage->lpData, _MAX_PATH);
			ClearAll();
			
			if(g_szInputDiffPath[0] != TEXT('\0'))
			{
				INT32 iRetCode = ParseDiffFile(g_szInputDiffPath);

				switch(iRetCode)
				{
				case WeePlugin::E_READ_FILE:
					m_gui->DisplayMessageBox(GetPluginInfo()->lpszPluginName, TEXT("Failed to read diff file!"), NULL, MBI_ERROR, MB_OK);
					break;

				case WeePlugin::E_INVALID_FORMAT:
					m_gui->DisplayMessageBox(GetPluginInfo()->lpszPluginName, TEXT("Invalid diff file format!"), NULL, MBI_ERROR, MB_OK);
					break;
				}
			}
		}
		break;
	case WeePlugin::NM_OUTPUT_CHANGED:
		_tcsncpy_s(g_szOutputExePath, (const TCHAR *)sMessage->lpData, _MAX_PATH);
		break;
	default:
			break;
	}
}

bool WeeDiffPlain::NeedDiffFile()
{
	return true;
}

INT32 WeeDiffPlain::PatchIt()
{
	FILE *fp;

	_tfopen_s(&fp, g_szInputExePath, TEXT("rb"));

	if(fp == NULL)
	{
		m_gui->DisplayMessageBox(GetPluginInfo()->lpszPluginName, TEXT("Failed to read input exe file!"), NULL, MBI_ERROR, MB_OK);
		return WeePlugin::E_READ_FILE;
	}

	struct _stat stFile;
	_tstat((const TCHAR *)g_szInputExePath, &stFile);

	if(stFile.st_size <= 0) // Hey, at least the MZ header should be there.. !
	{
		m_gui->DisplayMessageBox(GetPluginInfo()->lpszPluginName, TEXT("Input executable is empty!"), NULL, MBI_ERROR, MB_OK);
		return WeePlugin::E_EMPTY;
	}

	if(stFile.st_size > 10485760) // 10 MB should be more than enough.
	{
		m_gui->DisplayMessageBox(GetPluginInfo()->lpszPluginName, TEXT("Input executable is too large!"), NULL, MBI_ERROR, MB_OK);
		return WeePlugin::E_READ_FILE;
	}

	UCHAR *buffer = new UCHAR[stFile.st_size];

	if(buffer == NULL)
	{
		fclose(fp);
		m_gui->DisplayMessageBox(GetPluginInfo()->lpszPluginName, TEXT("Failed to allocate enough memory!"), NULL, MBI_ERROR, MB_OK);
		return WeePlugin::E_MEMORY;
	}

	if(fread(buffer, 1, stFile.st_size, fp) != stFile.st_size)
	{
		fclose(fp);
		delete[] buffer;
		m_gui->DisplayMessageBox(GetPluginInfo()->lpszPluginName, TEXT("Failed to read entire input exe!"), NULL, MBI_ERROR, MB_OK);
		return WeePlugin::E_READ_FILE;
	}

	fclose(fp);

	INT32 iCountPatched = 0;
	INT32 iCountFailed = 0;
	bool bIgnoreErrors = false;
	bool bInterrupt = false;
	INT32 iMsgCode = 0;
	TCHAR szMsg[256];

	ULONG32 uCRC = CRCCalcBuffer(buffer, stFile.st_size);

	ULONG32 uDTCRC = uCRC % 100000000; // DiffTeam CRC value
	ULONG32 uOCRC = Crc32ToOCrc(uCRC); // Maldiablo's CRC value

	if(uOCRC != m_iOriginalCRC && uDTCRC != m_iOriginalCRC)
	{
		_stprintf_s(szMsg, TEXT("CRC missmatch: Diff: 0x%X | Exe: 0x%X <> 0x%X. Continue anyway?"), m_iOriginalCRC, uDTCRC, uOCRC);
		iMsgCode = m_gui->DisplayMessageBox(GetPluginInfo()->lpszPluginName, szMsg, NULL, MBI_WARNING, MB_YESNO);
		if((iMsgCode & MB_TYPEMASK) == IDNO)
		{
			delete[] buffer;
			return WeePlugin::E_INVALID_EXE;
		}
	}	

	iMsgCode = 0;

	FILE *s_fp;

	// Don't care if failed to open/create file.
	_stprintf_s(szMsg, TEXT("%sWeeDiffPlain.Saved.txt"), g_szDllPath);

	_tfopen_s(&s_fp, szMsg, TEXT("w"));

	// TO-DO: Clean up this mess. :(
	for(WeePlugin::DiffItemList::iterator it = m_diffs.items.begin(); it != m_diffs.items.end(); it++)
	{
		if((*it)->uState & WeePlugin::ST_ENABLED)
		{
			DiffItemContainer *container = (DiffItemContainer *)(*it)->lpData;

			for(DiffItemContainer::iterator cit = container->begin(); cit != container->end(); cit++)
			{
				if((iMsgCode & MB_TYPEMASK) == IDNO)
				{
					bInterrupt = true;
					break;
				}
				else if(iMsgCode & IDCHECKBOX)
				{
					bIgnoreErrors = true;
				}

				if(cit->iOffset > stFile.st_size)
				{
					if(bIgnoreErrors == false)
					{
						_stprintf_s(szMsg, TEXT("%s : Offset 0x%X is out of bounds (0x%X)! Continue anyway?"), (*it)->lpszName, cit->iOffset, stFile.st_size);
						iMsgCode = m_gui->DisplayMessageBox(GetPluginInfo()->lpszPluginName, szMsg, g_lpszCheckboxMsg, MBI_QUESTION, MB_YESNO | MB_CHECKBOX);						
					}

					iCountFailed++;
					continue;
				}

				if(buffer[cit->iOffset] != cit->iOriginalValue)
				{
					if(bIgnoreErrors == false)
					{
						_stprintf_s(szMsg, TEXT("%s : Value missmatch (Exe: 0x%X; Val: 0x%X) at offset 0x%X! Apply anyway?"), (*it)->lpszName, buffer[cit->iOffset], cit->iOriginalValue, cit->iOffset);
						iMsgCode = m_gui->DisplayMessageBox(GetPluginInfo()->lpszPluginName, szMsg, g_lpszCheckboxMsg, MBI_QUESTION, MB_YESNO | MB_CHECKBOX);				
						if((iMsgCode & MB_TYPEMASK) == IDNO)
							continue;
					}
				}

				buffer[cit->iOffset] = cit->iReplaceValue;
			}

			if(bInterrupt == false)
			{
				iCountPatched++;
				
				if(s_fp != NULL)
				{
					_ftprintf_s(s_fp, TEXT("%s_%s_%s\n"), (*it)->lpType ? (*it)->lpType->lpszName : TEXT("X"), (*it)->lpGroup ? (*it)->lpGroup->lpszName : TEXT("Y"), (*it)->lpszName);
				}
			}
		}

		if(bInterrupt == true)
			break;
	}

	if(s_fp != NULL)
		fclose(s_fp);

	if(bInterrupt)
	{
		_stprintf_s(szMsg, TEXT("Patching aborted!"), iCountPatched, iCountFailed);
		m_gui->DisplayMessageBox(GetPluginInfo()->lpszPluginName, szMsg, NULL, MBI_WARNING, MB_OK);
	}
	else
	{
		_tfopen_s(&fp, g_szOutputExePath, TEXT("wb"));

		if(fp == NULL || fwrite(buffer, 1, stFile.st_size, fp) != stFile.st_size)
		{
			delete[] buffer;
			DeleteFile(g_szOutputExePath);
			m_gui->DisplayMessageBox(GetPluginInfo()->lpszPluginName, TEXT("Failed to save output exe!"), NULL, MBI_ERROR, MB_OK);
			return WeePlugin::E_WRITE_FILE;
		}

		fclose(fp);

		_stprintf_s(szMsg, TEXT("Applied %d and failed %d patches."), iCountPatched, iCountFailed);
		m_gui->DisplayMessageBox(GetPluginInfo()->lpszPluginName, szMsg, NULL, MBI_INFORMATION, MB_OK);
	}

	delete[] buffer;

	return 0;
}

INT32 WeeDiffPlain::GetDiffCount()
{
	return m_diffs.items.size();
}

WeePlugin::LPDIFFITEMLIST WeeDiffPlain::GetDiffItems()
{
	// You may think what sense it makes to declare
	// a member variable as protected and then return the address
	// to some function which isn't accessable at all. That's life and it sucks. :')
	return &m_diffs;
}

void WeeDiffPlain::ClearAll()
{
	for(WeePlugin::DiffTypeList::iterator it = m_diffs.types.begin(); it != m_diffs.types.end(); it++)
	{
		WeePlugin::DIFFTYPE *lpDiffType = (WeePlugin::DIFFTYPE *)(*it);

		if(lpDiffType->lpszName != NULL)
		{
			delete[] lpDiffType->lpszName;
			lpDiffType->lpszName = NULL;
		}

		delete lpDiffType;
	}

	m_diffs.types.clear();

	for(WeePlugin::DiffGroupList::iterator it = m_diffs.groups.begin(); it != m_diffs.groups.end(); it++)
	{
		WeePlugin::DIFFGROUP *lpDiffGroup = (WeePlugin::DIFFGROUP *)(*it);

		if(lpDiffGroup->lpszName != NULL)
		{
			delete[] lpDiffGroup->lpszName;
			lpDiffGroup->lpszName = NULL;
		}

		delete lpDiffGroup;
	}

	m_diffs.groups.clear();

	for(WeePlugin::DiffItemList::iterator it = m_diffs.items.begin(); it != m_diffs.items.end(); it++)
	{
		WeePlugin::DIFFITEM *lpDiffItem = (WeePlugin::DIFFITEM *)(*it);

		if(lpDiffItem->lpszName != NULL)
		{
			delete[] lpDiffItem->lpszName;
			lpDiffItem->lpszName = NULL;
		}

		if(lpDiffItem->lpData != NULL)
		{
			DiffItemContainer *lpDIC = (DiffItemContainer *)lpDiffItem->lpData;
			lpDIC->clear();

			delete lpDIC;
			lpDiffItem->lpData = NULL;
		}

		delete lpDiffItem;
	}

	m_diffs.items.clear();
}

/************************************************************************/
/* Please, if anyone has enough time, clean up this mess. Thank you!
/************************************************************************/
INT32 WeeDiffPlain::ParseDiffFile(LPCTSTR lpszDiffPath)
{
	TCHAR szLine[1024], szTmp[1024];
	TCHAR *szTmpPtr;

	std::vector<AUTOSAVE> autosaves;

	FILE *s_fp;
	_stprintf_s(szTmp, TEXT("%sWeeDiffPlain.Saved.txt"), g_szDllPath);	
	_tfopen_s(&s_fp, szTmp, TEXT("r"));

	if(s_fp != NULL)
	{
		while(_fgetts(szLine, sizeof(szLine), s_fp))
		{
			if(isspace(szLine[0]))
				continue;

			szTmpPtr = TrimSpaces(szLine);

			AUTOSAVE sAutoSave;
			UINT32 uLen = _tcslen(szTmpPtr);
			sAutoSave.lpszString = new TCHAR[uLen+1];
			_tcsncpy_s(sAutoSave.lpszString, uLen+1, szTmpPtr, uLen);
			sAutoSave.iHash = GenerateHash(sAutoSave.lpszString);

			autosaves.push_back(sAutoSave);
		}

		fclose(s_fp);
	}

	FILE *fp;
	_tfopen_s(&fp, lpszDiffPath, TEXT("r"));

	if(fp == NULL)
		return WeePlugin::E_READ_FILE;

	DiffItemContainer *container = NULL;

	// Clear previous member variables.
	m_iOriginalCRC = 0;
	ZeroMemory(&m_lpszBlurb, sizeof(m_lpszBlurb));

	bool bAutoSelectRecommended = m_gui->DisplayMessageBox(GetPluginInfo()->lpszPluginName, TEXT("Auto-Select recommended patches?"), NULL, MBI_QUESTION, MB_YESNO) == IDYES ? true : false;

	while(_fgetts(szLine, sizeof(szLine), fp))
	{
		TCHAR *str[4], *p = szLine;
		INT32 i;

		while(isspace(*p)) 
			p++;

		if(_tcslen(p) <= 0)
			continue;

		for(i = 0; i < 4; i++)
		{
			str[i] = p;

			if((p = _tcschr(str[i], TEXT(':'))) == NULL)
				break;

			*p = TEXT('\0');
			p++;
		}		

		switch(i+1)
		{
		case 2:
			if(_tcsicmp(str[0], TEXT("OCRC")) == 0)
			{
				m_iOriginalCRC = _ttoi(str[1]);
			} else if(_tcsicmp(str[0], TEXT("BLURB")) == 0)
			{
				_tcsncpy_s(m_lpszBlurb, str[1], 256);
			}

			// Ignore everything else
			// TO-DO: Logging

			break;
		case 4:
			// May add braces here? Uh..
			
			// I haven't seen a diff file that supports other types than byte_ after all.
			// So I assume that only this combination is valid: byte_<group>_<name>:offset:original:replace

			TCHAR *sp;

			if(_tcsnicmp(str[0], TEXT("byte_"), 5) != 0)
			{
				// TO-DO: Logging
				continue;
			}

			sp = str[0] + 5;
			if((p = _tcschr(sp, TEXT('_'))) == NULL)
			{
				// TO-DO: Logging
				continue;
			}

			*p = TEXT('\0');
			p++;
			
			// Now search for the group which is stored within type part.
			// Meh.. The following block needs optimization afterwards.

			TCHAR *tmp;
			tmp = sp;
			INT32 iTypeLen;
			INT32 iGroupLen;

			iTypeLen = 0;
			while(*tmp != TEXT('('))
			{
				iTypeLen++;
				tmp++;

				if(*tmp == TEXT('\0'))
					break;
			}

			WeePlugin::DIFFITEM *sDiff;
			sDiff = new WeePlugin::DIFFITEM;
			WeePlugin::DIFFTYPE *lpsType;
			lpsType = NULL;

			for(WeePlugin::DiffTypeList::iterator it = m_diffs.types.begin(); it != m_diffs.types.end(); it++)
			{
				if(_tcsnicmp(sp, (*it)->lpszName, iTypeLen) == 0)
				{
					lpsType = (WeePlugin::DIFFTYPE *)(*it);
				}
			}

			if(lpsType == NULL)
			{
				WeePlugin::DIFFTYPE *sType = new WeePlugin::DIFFTYPE;

				sType->iTypeId = m_diffs.types.size()+1;
				sType->lpszName = new TCHAR[iTypeLen+1];
				_tcsncpy_s(sType->lpszName, iTypeLen+1, sp, iTypeLen);
				sType->lpszName[iTypeLen] = TEXT('\0');

				m_diffs.types.push_back(sType);
				lpsType = &(*m_diffs.types.back());
			}

			sDiff->lpType = lpsType;

			WeePlugin::DIFFGROUP *lpsGroup;
			lpsGroup = NULL;

			// Hit the group part?
			iGroupLen = 0;
			if(*tmp != TEXT('\0'))
			{				
				while(*tmp != TEXT('\0'))
				{
					iGroupLen++;
					tmp++;
				}

				for(WeePlugin::DiffGroupList::iterator it = m_diffs.groups.begin(); it != m_diffs.groups.end(); it++)
				{
					if(_tcsnicmp(sp+iTypeLen, (*it)->lpszName, iGroupLen+1) == 0)
					{
						lpsGroup = (WeePlugin::DIFFGROUP *)(*it);
					}				
				}

				if(lpsGroup == NULL)
				{
					WeePlugin::DIFFGROUP *sGroup = new WeePlugin::DIFFGROUP;

					sGroup->iGroupId = m_diffs.groups.size()+1;
					sGroup->lpszName = new TCHAR[iGroupLen+1];
					_tcsncpy_s(sGroup->lpszName, iGroupLen+1, sp+iTypeLen, iGroupLen);
					sGroup->lpszName[iGroupLen] = TEXT('\0');

					m_diffs.groups.push_back(sGroup);
					lpsGroup = &(*m_diffs.groups.back());
				}
			}
			else
			{
				sDiff->lpGroup = NULL;
			}

			sDiff->lpGroup = lpsGroup;

			sp = p;

			while(*p != TEXT('\0'))
			{
				if(*p == TEXT('_'))
					*p = TEXT(' ');

				p++;
			}

			UINT32 iNameLen;
			iNameLen = _tcslen(sp)+1;
			sDiff->lpszName = new TCHAR[iNameLen];
			_tcsncpy_s(sDiff->lpszName, iNameLen, sp, iNameLen-1);

			sDiff->lpszDescription = NULL;

			if(bAutoSelectRecommended == true && stristr(sDiff->lpszName, TEXT("Recommended")) != NULL)
				sDiff->uState |= WeePlugin::ST_ENABLED;
			else
				sDiff->uState = 0;

			if((sDiff->uState & WeePlugin::ST_ENABLED) == 0)
			{
				_stprintf_s(szTmp, TEXT("%s_%s_%s"), sDiff->lpType ? sDiff->lpType->lpszName : TEXT("X"), sDiff->lpGroup ? sDiff->lpGroup->lpszName : TEXT("Y"), sDiff->lpszName);
				INT32 iHash = GenerateHash(szTmp);

				for(std::vector<AUTOSAVE>::iterator it = autosaves.begin(); it != autosaves.end(); it++)
				{
					if(iHash == it->iHash && _tcsicmp(szTmp, it->lpszString) == 0)
						sDiff->uState |= WeePlugin::ST_ENABLED;
				}
			}

			sDiff->iHash = GenerateHash(sDiff->lpszName);

			//////////////////////////////////////////////////////////////////////////
			
			container = FindDiffContainer(sDiff);

			// No matching diff "family" found? Create one.
			if(container == NULL)
			{
				sDiff->lpData = new DiffItemContainer;

				container = (DiffItemContainer *)sDiff->lpData;

				if(lpsType)
					lpsType->items.push_back(sDiff);

				if(lpsGroup)
					lpsGroup->items.push_back(sDiff);

				m_diffs.items.push_back(sDiff);
			}
			// If there already exists one, then delete the old and use the found result.
			// TO-DO: Probably create temporary local variable for this purpose instead of
			// allocating memory.
			else
			{
				delete[] sDiff->lpszName;
				delete sDiff;
			}

			PLAINDIFFITEM sPlainDiffItem;
			ZeroMemory(&sPlainDiffItem, sizeof(sPlainDiffItem));
			
			sPlainDiffItem.iOffset = _tcstol(str[1], NULL, 16);
			sPlainDiffItem.iOriginalValue = _tcstol(str[2], NULL, 10);
			sPlainDiffItem.iReplaceValue = _tcstol(str[3], NULL, 10);

			container->push_back(sPlainDiffItem);

			break;
		default:
			// TO-DO: Logging
			continue;
		}
	}

	for(std::vector<AUTOSAVE>::iterator it = autosaves.begin(); it != autosaves.end(); it++)
	{
		delete[] it->lpszString;
	}

	autosaves.clear();

	fclose(fp);

	return 0;
}

WeeDiffPlain::DiffItemContainer *WeeDiffPlain::FindDiffContainer(WeePlugin::DIFFITEM *lpsDiffItem)
{
	for(WeePlugin::DiffItemList::const_iterator it = m_diffs.items.begin(); it != m_diffs.items.end(); it++)
	{
		// As you may have already noticed the last check is useless at all since there's no way the previous checks
		// will result in wrong results! (*being naive*) But safe is safe. :(
		if((*it)->lpType == lpsDiffItem->lpType && (*it)->lpGroup == lpsDiffItem->lpGroup && (*it)->iHash == lpsDiffItem->iHash && _tcsicmp((*it)->lpszName, lpsDiffItem->lpszName) == 0)
		{
			return (DiffItemContainer *)(*it)->lpData;
		}
	}

	return NULL;
}

BOOL WINAPI DllMain(HINSTANCE hInstance,  DWORD fdwReason,  LPVOID lpvReserved)
{
	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH:
		GetModuleFileName(hInstance, g_szDllPath, _MAX_PATH);

		UINT32 i = _tcslen(g_szDllPath);

		while(g_szDllPath[i] != TEXT('\\'))
		{
			if(i-- == 0)
				break;
		}

		if(i != 0)
		{
			g_szDllPath[i+1] = TEXT('\0');
		}

		break;
	}

	return TRUE;
}


// Export the InitPlugin here with dllexport to simplify function export without a definition file.
extern "C" __declspec(dllexport) WeePlugin::IPlugin *InitPlugin(WeePlugin::IGUI *gui, USHORT unWeeDiffMajorVersion, USHORT unWeeDiffMinorVersion)
{
	// If you never saw a line like this, than stop viewing this file. :(
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );

	// Some kind of singleton pattern?
	// Used during debugging to prevent duplicate plug-in loading..
	// May still be useful in its own fashion.	
	if(g_WeeDiffPlain == NULL)
	{
		g_WeeDiffPlain = new WeeDiffPlain(gui);
		CRCGenerateTable();
	}

	return g_WeeDiffPlain;
}
