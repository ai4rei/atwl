/************************************************************************/
/*	WeeDiffGen.c
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

#include "WeeDiffGen.h"

// Reference to itself.
WeeDiffGen *g_WeeDiffGen = NULL;

// Contains the path to the executable.
TCHAR g_szDllPath[_MAX_PATH] = {0};
TCHAR g_szPluginPath[_MAX_PATH] = {0};
TCHAR g_szLogPath[_MAX_PATH] = {0};

/************************************************************************/
/* Random snippets
/************************************************************************/
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

/************************************************************************/
/* Cleanup everything.
/************************************************************************/
void WeeDiffGen::Release()
{
	ClearAll();
	g_WeeDiffGen = NULL;

	if(m_dgc != NULL)
	{
		delete m_dgc;
		m_dgc = NULL;
	}

	delete this;
}

/************************************************************************/
/* Function to free up memory allocated by this plug-in.
/************************************************************************/
void WeeDiffGen::Free(LPVOID memory)
{
	delete memory;
	memory = NULL;
}

/************************************************************************/
/* Display settings.
/************************************************************************/
void WeeDiffGen::About(HWND hParent)
{
	m_gui->DisplayMessageBox(GetPluginInfo()->lpszPluginName, TEXT("A diff generator for Ragnarok clients that were compiled with VC9. Copyright (C) 2011 Shinryo"), NULL, MBI_INFORMATION, MB_OK);
}

/************************************************************************/
/* Return information about this plug-in.
/************************************************************************/
WeePlugin::LPWEEPLUGININFO WeeDiffGen::GetPluginInfo()
{
	static WeePlugin::WEEPLUGININFO wpi = 
	{
		TEXT("WeeDiffGenerator v1.0.2"),
		TEXT("Shinryo"),
		1,
		1,
		{ 0x62c8fb7a, 0xa729, 0x4014, { 0xbb, 0x67, 0x10, 0xc7, 0x46, 0x38, 0xb7, 0x64 } }
	};

	return &wpi;
}

/************************************************************************/
/* Not diff file needed.
/************************************************************************/
LPCTSTR WeeDiffGen::GetSupportedFormat()
{
	return NULL;
}

/************************************************************************/
/* What else?
/************************************************************************/
LPCTSTR WeeDiffGen::GetDiffTitle()
{
	if(m_ragexe != NULL)
	{
		_stprintf_s(m_szDiffTitle, _MAX_PATH, TEXT("WeeDiffGenerator :: %d"), m_ragexe->GetClientDate());
	}
	else
	{
		_stprintf_s(m_szDiffTitle, _MAX_PATH, TEXT("WeeDiffGenerator"));
	}
	
	return m_szDiffTitle;
}

/************************************************************************/
/*                                                                      */
/************************************************************************/
void WeeDiffGen::Notify(WeePlugin::LPNOTIFYMESSAGE sMessage)
{
	switch(sMessage->iMessage)
	{
	case WeePlugin::NM_EXE_CHANGED:

		ClearAll();

		if(((LPCTSTR)sMessage->lpData)[0] == TEXT('\0'))
			return;

		try
		{
			m_ragexe = new RagExe((LPCTSTR)sMessage->lpData);
			m_wpm = new WeePluginManager();			
			if(InitPlugins(g_szPluginPath) < 0)
			{
				throw TEXT("Cannot find any WeeDiffGen plug-ins. Please create a folder named 'WeeDiffGen' inside the same folder where the plug-in is.");
			}
			else
			{
				ParseDiffs();
			}
		}
		catch (LPCTSTR lpszMessage)
		{
			ClearAll();
			m_gui->DisplayMessageBox(GetPluginInfo()->lpszPluginName, lpszMessage, NULL, MBI_ERROR, MB_OK);
		}

		break;
	case WeePlugin::NM_OUTPUT_CHANGED:
		_tcsncpy_s(m_szOutputExe, (const TCHAR *)sMessage->lpData, _MAX_PATH);
		break;
	case WeePlugin::NM_ITEMPRESELECTED:
		{
			WeePlugin::DIFFITEM *lpDiffItem = (WeePlugin::DIFFITEM *)sMessage->lpData;
			lpDiffItem->uState |= WeePlugin::ST_ENABLED;

			WeeDiffGenPlugin::IWDGPlugin *lpPlugin = (WeeDiffGenPlugin::IWDGPlugin *)lpDiffItem->lpData;

			// TO-DO: Maybe throw error here.
			if(lpPlugin == NULL)
				return;

			sMessage->iRetCode = lpPlugin->Enabled();
		}
		break;
	case WeePlugin::NM_ITEMPREDESELECTED:
		{
			WeePlugin::DIFFITEM *lpDiffItem = (WeePlugin::DIFFITEM *)sMessage->lpData;
			if(lpDiffItem->uState & WeePlugin::ST_ENABLED)
				lpDiffItem->uState ^= WeePlugin::ST_ENABLED;

			WeeDiffGenPlugin::IWDGPlugin *lpPlugin = (WeeDiffGenPlugin::IWDGPlugin *)lpDiffItem->lpData;

			// TO-DO: Maybe throw error here.
			if(lpPlugin == NULL)
				return;

			sMessage->iRetCode = lpPlugin->Disabled();
		}
		break;
	}
}

/************************************************************************/
/* This generator does not need any diff files.
/************************************************************************/
bool WeeDiffGen::NeedDiffFile()
{
	return false;
}

/************************************************************************/
/* Apply selected patches.
/************************************************************************/
INT32 WeeDiffGen::PatchIt()
{
	if(m_ragexe == NULL)
	{
		m_gui->DisplayMessageBox(GetPluginInfo()->lpszPluginName, TEXT("There is no executable loaded!"), NULL, MBI_ERROR, MB_OK);
		return WeePlugin::E_EMPTY;
	}

	UCHAR *buffer = new UCHAR[m_ragexe->m_uExeSize];

	if(buffer == NULL)
	{
		m_gui->DisplayMessageBox(GetPluginInfo()->lpszPluginName, TEXT("Failed to allocate enough memory!"), NULL, MBI_ERROR, MB_OK);
		return WeePlugin::E_MEMORY;
	}

	memcpy(buffer, m_ragexe->m_cExeBuffer, m_ragexe->m_uExeSize);

	TCHAR szMsg[256];
	UINT32 uCountPatches = 0;
	UINT32 uCountFails = 0;

	FILE *s_fp;

	// Don't care if failed to open/create file.
	_stprintf_s(szMsg, TEXT("%sWeeDiffGen.saved.bin"), g_szDllPath);
	_tfopen_s(&s_fp, szMsg, TEXT("wb"));

	for(WeePlugin::DiffItemList::iterator it = m_diffs.items.begin(); it != m_diffs.items.end(); it++)
	{
		if((*it)->uState & WeePlugin::ST_ENABLED)
		{
			WeeDiffGenPlugin::IWDGPlugin *lpPlugin = (WeeDiffGenPlugin::IWDGPlugin *)(*it)->lpData;
			WeeDiffGenPlugin::DiffData *lpDiffData = lpPlugin->GetDiffData();

			if(lpDiffData == NULL)
			{
				uCountFails++;
				continue;
			}

			for(WeeDiffGenPlugin::DiffData::iterator it = lpDiffData->begin(); it != lpDiffData->end(); it++)
			{
				buffer[it->iOffset] = it->iReplaceValue;
			}

			uCountPatches++;

			if(s_fp != NULL)
			{
				fwrite(&lpPlugin->GetPluginInfo()->guid, 1, sizeof(GUID), s_fp);
			}
		}
	}

	fclose(s_fp);

	FILE *fp;

	_tfopen_s(&fp, m_szOutputExe, TEXT("wb"));

	if(fp == NULL || fwrite(buffer, 1, m_ragexe->m_uExeSize, fp) != m_ragexe->m_uExeSize)
	{
		delete[] buffer;
		DeleteFile(m_szOutputExe);
		m_gui->DisplayMessageBox(GetPluginInfo()->lpszPluginName, TEXT("Failed to save output exe!"), NULL, MBI_ERROR, MB_OK);
		return WeePlugin::E_WRITE_FILE;
	}

	fclose(fp);

	_stprintf_s(szMsg, TEXT("Applied %d and failed %d patches."), uCountPatches, uCountFails);
	m_gui->DisplayMessageBox(GetPluginInfo()->lpszPluginName, szMsg, NULL, MBI_INFORMATION, MB_OK);

	delete[] buffer;

	return 0;
}

/************************************************************************/
/* Deprecated.
/************************************************************************/
INT32 WeeDiffGen::GetDiffCount()
{
	return m_diffs.items.size();
}

/************************************************************************/
/* Return pointer to the diff item list.
/************************************************************************/
WeePlugin::LPDIFFITEMLIST WeeDiffGen::GetDiffItems()
{
	return &m_diffs;
}

/************************************************************************/
/* Don't prevent collisions
/************************************************************************/
bool WeeDiffGen::PreventGroupCollision()
{
	return false;
}

WeePlugin::ColumnNames *WeeDiffGen::GetColumnNames()
{
	static WeePlugin::ColumnNames ColNames;

	if(ColNames.size() <= 0)
	{
		ColNames.push_back(TEXT("Value"));
		ColNames.push_back(TEXT("Note"));
		ColNames.push_back(TEXT("PL Author"));
	}

	return &ColNames;
}

void WeeDiffGen::GetColumnDispInfo(WeePlugin::DIFFITEM *lpDiffItem, NMLVDISPINFO *lpDispInfo)
{
	WeeDiffGenPlugin::IWDGPlugin *lpPlugin = (WeeDiffGenPlugin::IWDGPlugin *)lpDiffItem->lpData;

	switch(lpDispInfo->item.iSubItem)
	{
	case 5:
		{
			LPTSTR lpszStr = (LPTSTR)lpPlugin->GetInputValue();

			if(lpszStr != NULL)
				lpDispInfo->item.pszText = lpszStr;
		}
		break;
	case 6:
		{
			LPTSTR lpszStr = (LPTSTR)lpPlugin->GetPluginInfo()->lpszNote;

			if(lpszStr != NULL)
				lpDispInfo->item.pszText = lpszStr;
		}
		break;
	case 7:
		lpDispInfo->item.pszText = lpPlugin->GetPluginInfo()->lpszAuthorName;
		break;
	}
}

/************************************************************************/
/* Used to save path to the dll.
/************************************************************************/
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
			g_szDllPath[i+1] = TEXT('\0');

		_stprintf_s(g_szPluginPath, _MAX_PATH, TEXT("%sWeeDiffGen\\"), g_szDllPath);
		_stprintf_s(g_szLogPath, _MAX_PATH, TEXT("%sWeeDiffGen.log"), g_szPluginPath);

		break;
	}

	return TRUE;
}

/************************************************************************/
/* Create plug-in instance.
/************************************************************************/
extern "C" __declspec(dllexport) WeePlugin::IPlugin *InitPlugin(WeePlugin::IGUI *gui, USHORT unWeeDiffMajorVersion, USHORT unWeeDiffMinorVersion)
{
	// Enable functions to track down memory leaks
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	//_CrtSetBreakAlloc(52);

	if(g_WeeDiffGen == NULL)
	{
		g_WeeDiffGen = new WeeDiffGen(gui);
	}

	return g_WeeDiffGen;
}

/************************************************************************/
/*                                                                      */
/************************************************************************/
void WeeDiffGen::ClearAll()
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

		if(lpDiffItem->lpszDescription != NULL)
		{
			delete[] lpDiffItem->lpszDescription;
			lpDiffItem->lpszDescription = NULL;
		}

		if(lpDiffItem->lpData != NULL)
		{
			
		}

		delete lpDiffItem;
	}

	m_diffs.items.clear();

	if(m_ragexe != NULL)
	{
		delete m_ragexe;
		m_ragexe = NULL;
	}

	if(m_wpm != NULL)
	{
		delete m_wpm;
		m_wpm = NULL;
	}
}

INT32 WeeDiffGen::InitPlugins(LPCTSTR lpszDirectory)
{
	HANDLE hFind = INVALID_HANDLE_VALUE;
	WIN32_FIND_DATA fdata;

	TCHAR szSearchPath[_MAX_PATH+2], szFullPathName[_MAX_PATH];

	INT32 iLen = _tcslen(lpszDirectory);
	UINT32 uCount = 0;

	if(iLen >= _MAX_PATH)
		return -1;

	bool bIsRootDirectory = (lpszDirectory[iLen-1] == TEXT('\\'));

	if(bIsRootDirectory)
		_stprintf_s(szSearchPath, TEXT("%s*.dll"), lpszDirectory);
	else
		_stprintf_s(szSearchPath, TEXT("%s\\*.dll"), lpszDirectory);

	hFind = FindFirstFile(szSearchPath, &fdata);

	if(hFind != INVALID_HANDLE_VALUE)
	{
		do {
			if(bIsRootDirectory)
				_stprintf_s(szFullPathName, TEXT("%s%s"), lpszDirectory, fdata.cFileName);
			else
				_stprintf_s(szFullPathName, TEXT("%s\\%s"), lpszDirectory, fdata.cFileName);

			INT32 iRetCode = m_wpm->LoadPlugin(szFullPathName, m_dgc, 1, 0);

			if(iRetCode != WeePluginManager::SUCCESS)
			{
				// TO-DO: Logging when failed to load plug-in file.
				continue;
			}

		} while(FindNextFile(hFind, &fdata) != 0);

	} else
	{
		return -1;
	}

	FindClose(hFind);
	hFind = INVALID_HANDLE_VALUE;

	return 0;
}

void WeeDiffGen::ParseDiffs()
{
	TCHAR szTmp[_MAX_PATH];
	UINT32 iLen;
	std::vector<GUID> autosaves;

	FILE *s_fp;
	_stprintf_s(szTmp, TEXT("%sWeeDiffGen.saved.bin"), g_szDllPath);	
	_tfopen_s(&s_fp, szTmp, TEXT("rb"));

	if(s_fp != NULL)
	{
		GUID sGuid;

		while(fread(&sGuid, 1, sizeof(GUID), s_fp) == sizeof(GUID))
		{			
			autosaves.push_back(sGuid);
		}

		fclose(s_fp);
	}

	bool bAutoSelectRecommended = m_gui->DisplayMessageBox(GetPluginInfo()->lpszPluginName, TEXT("Auto-Select recommended patches?"), NULL, MBI_QUESTION, MB_YESNO) == IDYES ? true : false;

	for(WeePluginManager::PluginMap::const_iterator it = m_wpm->m_plugins.begin(); it != m_wpm->m_plugins.end(); it++)
	{
		WeeDiffGenPlugin::LPWDGPLUGININFO lpWDGPluginInfo = (WeeDiffGenPlugin::LPWDGPLUGININFO)it->second.lpInterface->GetPluginInfo();

		WeeDiffGenPlugin::DiffData *lpDiffData = it->second.lpInterface->GeneratePatch();

		if(lpDiffData == NULL)
			continue;

		WeePlugin::DIFFITEM *sDiff = new WeePlugin::DIFFITEM;
		WeePlugin::DIFFTYPE *lpsType = NULL;
		WeePlugin::DIFFGROUP *lpsGroup = NULL;

		if(lpWDGPluginInfo->lpszDiffType != NULL)
		{
			for(WeePlugin::DiffTypeList::iterator t_it = m_diffs.types.begin(); t_it != m_diffs.types.end(); t_it++)
			{
				if(_tcsicmp(lpWDGPluginInfo->lpszDiffType, (*t_it)->lpszName) == 0)
				{
					lpsType = (WeePlugin::DIFFTYPE *)(*t_it);
				}
			}

			if(lpsType == NULL)
			{
				WeePlugin::DIFFTYPE *sType = new WeePlugin::DIFFTYPE;

				iLen = _tcslen(lpWDGPluginInfo->lpszDiffType);

				sType->iTypeId = m_diffs.types.size() + 1;
				sType->lpszName = new TCHAR[iLen + 1];
				_tcscpy_s(sType->lpszName, iLen + 1, lpWDGPluginInfo->lpszDiffType);

				m_diffs.types.push_back(sType);
				lpsType = &(*m_diffs.types.back());
			}

			sDiff->lpType = lpsType;
		}

		if(lpWDGPluginInfo->lpszDiffGroup != NULL)
		{
			for(WeePlugin::DiffGroupList::iterator g_it = m_diffs.groups.begin(); g_it != m_diffs.groups.end(); g_it++)
			{
				if(_tcsicmp(lpWDGPluginInfo->lpszDiffGroup, (*g_it)->lpszName) == 0)
				{
					lpsGroup = (WeePlugin::DIFFGROUP *)(*g_it);
				}				
			}

			if(lpsGroup == NULL)
			{
				WeePlugin::DIFFGROUP *sGroup = new WeePlugin::DIFFGROUP;

				iLen = _tcslen(lpWDGPluginInfo->lpszDiffGroup);

				sGroup->iGroupId = m_diffs.groups.size() + 1;
				sGroup->lpszName = new TCHAR[iLen + 1];
				_tcscpy_s(sGroup->lpszName, iLen + 1, lpWDGPluginInfo->lpszDiffGroup);

				m_diffs.groups.push_back(sGroup);

				lpsGroup = &(*m_diffs.groups.back());
			}

			sDiff->lpGroup = lpsGroup;
		}

		if(lpWDGPluginInfo->lpszDiffDesc != NULL)
		{
			iLen = _tcslen(lpWDGPluginInfo->lpszDiffDesc);
			sDiff->lpszDescription = new TCHAR[iLen + 1];
			_tcscpy_s(sDiff->lpszDescription, iLen + 1, lpWDGPluginInfo->lpszDiffDesc);
		}

		iLen = _tcslen(lpWDGPluginInfo->lpszDiffName) + 1;
		sDiff->lpszName = new TCHAR[iLen + 1];
		_tcscpy_s(sDiff->lpszName, iLen + 1, lpWDGPluginInfo->lpszDiffName);

		sDiff->uState = 0;

		sDiff->iHash = GenerateHash(sDiff->lpszName);

		if(lpsType != NULL)
		{
			lpsType->items.push_back(sDiff);
		}

		if(lpsGroup != NULL)
		{
			lpsGroup->items.push_back(sDiff);
		}

		for(std::vector<GUID>::iterator its = autosaves.begin(); its != autosaves.end(); its++)
		{
			if(memcmp(&(*its), &lpWDGPluginInfo->guid, sizeof(GUID)) == 0)
			{
				sDiff->uState |= WeePlugin::ST_ENABLED;
				break;
			}
		}

		if((sDiff->uState &  WeePlugin::ST_ENABLED) == 0 && bAutoSelectRecommended == true && lpWDGPluginInfo->lpszNote != NULL && stristr(lpWDGPluginInfo->lpszNote, TEXT("Recommended")) != NULL)
			sDiff->uState |= WeePlugin::ST_ENABLED;

		sDiff->lpData = (LPVOID)&*it->second.lpInterface;

		m_diffs.items.push_back(sDiff);
	}
}