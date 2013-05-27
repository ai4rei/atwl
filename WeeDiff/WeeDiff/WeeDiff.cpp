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

#include <stdlib.h>
#include <crtdbg.h>

#ifdef _DEBUG
#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#define new DEBUG_NEW
#endif

//#define HEADER_CHECKBOX

#pragma warning(disable: 4996)

#include "WeeDiff.h"

#include "resource.h"
#include "weeresource.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <Windows.h>
#include <Windowsx.h>
#include <CommDlg.h>

#include "WeePluginManager.h"
#include "WeeGUInterface.h"

#include "..\..\Snippets\Window.h"

#include "..\..\Snippets\WeeMessageBox.h"
#include "..\..\Snippets\WeeException.h"
#include "..\..\Snippets\ListView.h"
#include "..\Common\WeeDiffPlugin.h"

WeeTools::Window *g_window = NULL;
WeePluginManager *g_wpm = NULL;
WeeGUI *g_wgui = NULL;
WeePluginManager::IBasePlugin *g_activePlugin = NULL;

HINSTANCE g_hInstance = NULL;
HBRUSH g_hBrush = GetSysColorBrush(CTLCOLOR_DLG);
NONCLIENTMETRICS g_ncm;
HDC g_hMemDC = NULL;
WNDPROC g_OldListViewProc;

INT_PTR CALLBACK AboutDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void OnRefresh();
INT32 OnMainNotify(HWND hWnd, INT32 iCmdID, LPNMHDR lpNotify);

TCHAR g_lpszHeading[_MAX_PATH];
LPTSTR g_lpszNoDiffSelected = TEXT("[ no diff selected ]");
LPTSTR g_strSelectPatchEngine = TEXT("Select patch engine:");
LPTSTR g_strOutputExecutable = TEXT("Output executable:");
LPTSTR g_strSourceDiffFile = TEXT("Source diff patch:");
LPTSTR g_strSourceExeFile = TEXT("Source executable:");

TCHAR g_szInputExePath[_MAX_PATH] = {0};
TCHAR g_szInputDiffPath[_MAX_PATH] = {0};
TCHAR g_szOutputExePath[_MAX_PATH] = {0};

TCHAR g_szPatcherPath[_MAX_PATH] = {0};

UINT32 g_uCustomColumns = 0;

INT CALLBACK ListViewCompareName(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	WeePlugin::DIFFITEM *lpDiffItem1 = (WeePlugin::DIFFITEM *)lParam1;
	WeePlugin::DIFFITEM *lpDiffItem2 = (WeePlugin::DIFFITEM *)lParam2;

	return _tcsicmp(lpDiffItem1->lpszName, lpDiffItem2->lpszName);
}

INT CALLBACK ListViewCompareGroup(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	WeePlugin::DIFFITEM *lpDiffItem1 = (WeePlugin::DIFFITEM *)lParam1;
	WeePlugin::DIFFITEM *lpDiffItem2 = (WeePlugin::DIFFITEM *)lParam2;

	LPTSTR cmpStr1 = TEXT("");
	LPTSTR cmpStr2 = TEXT("");

	if(lpDiffItem1->lpGroup)
		cmpStr1 = lpDiffItem1->lpGroup->lpszName;

	if(lpDiffItem2->lpGroup)
		cmpStr2 = lpDiffItem2->lpGroup->lpszName;

	INT32 CompareA = _tcsicmp(cmpStr1, cmpStr2);
	INT32 CompareB = ListViewCompareName(lParam1, lParam2, lParamSort);

	return CompareA == 0 ? CompareB : CompareA;
}

INT CALLBACK ListViewCompareType(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	WeePlugin::DIFFITEM *lpDiffItem1 = (WeePlugin::DIFFITEM *)lParam1;
	WeePlugin::DIFFITEM *lpDiffItem2 = (WeePlugin::DIFFITEM *)lParam2;

	LPTSTR cmpStr1 = TEXT("");
	LPTSTR cmpStr2 = TEXT("");

	if(lpDiffItem1->lpType)
		cmpStr1 = lpDiffItem1->lpType->lpszName;

	if(lpDiffItem2->lpType)
		cmpStr2 = lpDiffItem2->lpType->lpszName;

	INT32 CompareA = _tcsicmp(cmpStr1, cmpStr2);
	INT32 CompareB = ListViewCompareGroup(lParam1, lParam2, lParamSort);

	return CompareA == 0 ? CompareB : CompareA;
}

inline WeePluginManager::LPPLUGIN GetActivePlugin()
{
	return (WeePluginManager::LPPLUGIN)ComboBox_GetItemData(g_window->GetControl(IDC_COMBOBOX_PLUGIN)->GetHandle(), ComboBox_GetCurSel(g_window->GetControl(IDC_COMBOBOX_PLUGIN)->GetHandle()));
}

bool CheckActivePluginVersion(UINT32 uMajor, UINT32 uMinor)
{
	WeePlugin::LPWEEPLUGININFO lpPluginInfo = GetActivePlugin()->lpInterface->GetPluginInfo();

	if(lpPluginInfo->unMinMajorVersion > uMajor || (lpPluginInfo->unMinMajorVersion == uMajor && lpPluginInfo->unMinMinorVersion >= uMinor))
		return true;

	return false;
}

void FillSupportedDiffs(TCHAR **p)
{
	for(WeePluginManager::PluginMap::const_iterator it = g_wpm->m_plugins.begin(); it != g_wpm->m_plugins.end(); it++)
	{
		if(it->second.lpInterface->NeedDiffFile())
		{
			LPCTSTR lpszRef = it->second.lpInterface->GetSupportedFormat();

			if(it != g_wpm->m_plugins.begin())
			{
				**p = TEXT(';');
				(*p)++;
			}

			**p = TEXT('*');
			(*p)++;

			for(UINT32 i = 0; i < _tcslen(lpszRef); i++, (*p)++)
			{
				**p = lpszRef[i];
			}
		}
		
	}
}

INT32 DisplayMessageBox(LPCTSTR lpszCaption, LPCTSTR lpszText, LPCTSTR lpszCheckbox, UINT32 uIcondIndex, INT32 iStyle)
{
	MSGBOXINFO mbi;

	mbi.hInstance = g_hInstance;
	mbi.lpCaption = lpszCaption;
	mbi.lpText = lpszText;
	mbi.lpCheckboxText = lpszCheckbox;
	mbi.uIconIndex = uIcondIndex;
	mbi.iStyle = iStyle;
	mbi.uIconResourceID = IDB_WMB;

	return WeeMessageBox(g_window->GetHandle(), MAKEINTRESOURCE(IDD_MESSAGEBOX), &mbi);
}

void DisplayException(LPCTSTR errorMsg, SHORT nType)
{
	HWND hParent = NULL;
	TCHAR *szTitle;

	TCHAR szInfo[] = TEXT("Information");
	TCHAR szWarning[] = TEXT("Warning");
	TCHAR szError[] = TEXT("Error");

	if (g_window != NULL)
		hParent = g_window->GetHandle();

	switch(nType)
	{
	case WeeException::E_ERROR:
		nType = MBI_ERROR;
		szTitle = szError;
		break;
	case WeeException::E_INFO:
		nType = MBI_INFORMATION;
		szTitle = szInfo;
		break;
	case WeeException::E_WARNING:
		nType = MBI_WARNING;
		szTitle = szWarning;
		break;
	}

	DisplayMessageBox(szTitle, errorMsg, NULL, nType, MB_OK);
}

INT32 InitPlugins(LPCTSTR lpszDirectory)
{
	HANDLE hFind = INVALID_HANDLE_VALUE;
	WIN32_FIND_DATA fdata;
	HWND hComboBox = g_window->GetControl(IDC_COMBOBOX_PLUGIN)->GetHandle();

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

			INT32 iRetCode = g_wpm->LoadPlugin(szFullPathName, g_wgui, APP_VMAJOR, APP_VMINOR);

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

	for(WeePluginManager::PluginMap::const_iterator it = g_wpm->m_plugins.begin(); it != g_wpm->m_plugins.end(); it++)
	{
		INT32 iIndex = ComboBox_AddString(hComboBox, it->second.lpInterface->GetPluginInfo()->lpszPluginName);

		if(iIndex >= 0)
		{
			ComboBox_SetItemData(hComboBox, iIndex, (LPARAM)&it->second);
		}
	}

	if(ComboBox_GetCount(hComboBox) <= 0)
		return -1;

	ComboBox_SetCurSel(hComboBox, 0);
	OnRefresh();
	
	return 0;
}

void OnMainCreate(HWND hWnd, LPCREATESTRUCT cs)
{
	WeeTools::Window::Center(hWnd);
}

void OnMainDestroy(HWND hWnd)
{

}

void OnMainSize(HWND hWnd, INT32 iWidth, INT32 iHeight)
{
	HWND hCtrlWnd = NULL;

	RECT rcWindow, rcTrack;
	SIZE szWindow;

	ZeroMemory(&rcWindow, sizeof(RECT));
	ZeroMemory(&szWindow, sizeof(SIZE));
	ZeroMemory(&rcTrack, sizeof(RECT));

	GetClientRect(hWnd, &rcWindow);
	szWindow.cx = rcWindow.right - rcWindow.left;
	szWindow.cy = rcWindow.bottom - rcWindow.top;

	// Plugin combobox
	rcTrack.left = 10;
	rcTrack.bottom = 20;
	rcTrack.top = szWindow.cy - rcTrack.bottom - 20;
	rcTrack.right = szWindow.cx - 280 - rcTrack.left;

	hCtrlWnd = g_window->GetControl(IDC_COMBOBOX_PLUGIN)->GetHandle();
	SetWindowPos(hCtrlWnd, NULL, rcTrack.left, rcTrack.top, rcTrack.right, rcTrack.bottom, 0);

	hCtrlWnd = g_window->GetControl(ID_BTN_PATCHIT)->GetHandle();
	SetWindowPos(hCtrlWnd, NULL, szWindow.cx - 80 - 10, rcTrack.top - 1, 80, rcTrack.bottom + 5, 0);

	hCtrlWnd = g_window->GetControl(ID_ABOUT)->GetHandle();
	SetWindowPos(hCtrlWnd, NULL, szWindow.cx - 80 * 2 - 10 * 2, rcTrack.top - 1, 80, rcTrack.bottom + 5, 0);

	hCtrlWnd = g_window->GetControl(ID_ABOUT_PLUGIN)->GetHandle();
	SetWindowPos(hCtrlWnd, NULL, szWindow.cx - 80 * 3 - 10 * 3, rcTrack.top - 1, 80, rcTrack.bottom + 5, 0);

	// Output Exe Edit
	rcTrack.top -= 30 + rcTrack.bottom;
	rcTrack.right += 180;

	hCtrlWnd = g_window->GetControl(IDC_EDIT_INPUT_OUT)->GetHandle();
	SetWindowPos(hCtrlWnd, NULL, rcTrack.left, rcTrack.top, rcTrack.right, rcTrack.bottom, 0);

	hCtrlWnd = g_window->GetControl(ID_BTN_INPUT_OUT)->GetHandle();
	SetWindowPos(hCtrlWnd, NULL, szWindow.cx - 80 - 10, rcTrack.top - 1, 80, rcTrack.bottom + 1, 0);

	// Input Diff Edit
	rcTrack.top -= 30 + rcTrack.bottom;

	hCtrlWnd = g_window->GetControl(IDC_EDIT_INPUT_DIFF)->GetHandle();
	SetWindowPos(hCtrlWnd, NULL, rcTrack.left, rcTrack.top, rcTrack.right, rcTrack.bottom, 0);

	hCtrlWnd = g_window->GetControl(ID_BTN_INPUT_DIFF)->GetHandle();
	SetWindowPos(hCtrlWnd, NULL, szWindow.cx - 80 - 10, rcTrack.top - 1, 80, rcTrack.bottom + 1, 0);

	// Input Exe Edit
	rcTrack.top -= 30 + rcTrack.bottom;

	hCtrlWnd = g_window->GetControl(IDC_EDIT_INPUT_EXE)->GetHandle();
	SetWindowPos(hCtrlWnd, NULL, rcTrack.left, rcTrack.top, rcTrack.right, rcTrack.bottom, 0);

	hCtrlWnd = g_window->GetControl(ID_BTN_INPUT_EXE)->GetHandle();
	SetWindowPos(hCtrlWnd, NULL, szWindow.cx - 80 - 10, rcTrack.top - 1, 80, rcTrack.bottom + 1, 0);

	// Listview
	rcTrack.top -= 30 * 2 + rcTrack.bottom;

	rcTrack.right = szWindow.cx - rcTrack.left * 2;
	rcTrack.bottom = rcTrack.top;
	rcTrack.top = 50;

	hCtrlWnd = g_window->GetControl(IDC_LISTVIEW_AVAILPATCHES)->GetHandle();
	SetWindowPos(hCtrlWnd, NULL, rcTrack.left, rcTrack.top, rcTrack.right, rcTrack.bottom, 0);
}

bool OnKeydown(HWND hWnd, WPARAM vKey)
{
	if(vKey == 'A' && HIBYTE(GetKeyState(VK_CONTROL)))
	{
		HWND hListView = g_window->GetControl(IDC_LISTVIEW_AVAILPATCHES)->GetHandle();

		for(INT32 i = 0; i < ListView_GetItemCount(hListView); i++)
		{
			ListView_SetCheckState(hListView, i, TRUE);
		}

		return true;
	} else if(vKey == 'D' && HIBYTE(GetKeyState(VK_CONTROL)))
	{
		HWND hListView = g_window->GetControl(IDC_LISTVIEW_AVAILPATCHES)->GetHandle();

		for(INT32 i = 0; i < ListView_GetItemCount(hListView); i++)
		{
			ListView_SetCheckState(hListView, i, FALSE);
		}

		return true;
	} else if(vKey == 'C' && HIBYTE(GetKeyState(VK_CONTROL)))
	{


		if(OpenClipboard(g_window->GetHandle()))
		{
			WeePlugin::LPDIFFITEMLIST lpDiffItems = GetActivePlugin()->lpInterface->GetDiffItems();

			EmptyClipboard();
			HGLOBAL hClipboardData;
			hClipboardData = GlobalAlloc(GMEM_MOVEABLE, lpDiffItems->items.size() * 512);

			if(hClipboardData != NULL)
			{
				TCHAR * pchData;
				pchData = (TCHAR*)GlobalLock(hClipboardData);

				ZeroMemory(pchData, lpDiffItems->items.size() * 512);

				NMLVDISPINFO sNmLvDispInfo = {0};
				sNmLvDispInfo.hdr.code = LVN_GETDISPINFO;
				sNmLvDispInfo.hdr.idFrom = IDC_LISTVIEW_AVAILPATCHES;
				sNmLvDispInfo.hdr.hwndFrom = g_window->GetControl(IDC_LISTVIEW_AVAILPATCHES)->GetHandle();

				LVCOLUMN lvColumn = {0};
				TCHAR szColumnName[256];
				lvColumn.mask = LVCF_TEXT;
				lvColumn.pszText = szColumnName;
				lvColumn.cchTextMax = sizeof(TCHAR) * 256;

				HWND hListView = g_window->GetControl(IDC_LISTVIEW_AVAILPATCHES)->GetHandle();

				INT32 iItemCount = ListView_GetItemCount(g_window->GetControl(IDC_LISTVIEW_AVAILPATCHES)->GetHandle());
				for(INT32 i = 0; i < iItemCount; i++)
				{
					if(ListView_GetCheckState(g_window->GetControl(IDC_LISTVIEW_AVAILPATCHES)->GetHandle(), i) == TRUE)
					{
						LVITEM lvItem = {0};
						lvItem.iItem = i;
						lvItem.mask = LVIF_PARAM;

						ListView_GetItem(g_window->GetControl(IDC_LISTVIEW_AVAILPATCHES)->GetHandle(), &lvItem);
						WeePlugin::DIFFITEM *lpDiffItem = (WeePlugin::DIFFITEM *)lvItem.lParam;

						UINT32 uColumnCount = 5 + g_uCustomColumns;
						UINT32 uLen = 0;

						for(UINT32 i = 1; i < uColumnCount; i++)
						{						
							ListView_GetColumn(hListView, i, &lvColumn);
							
							sNmLvDispInfo.item.lParam = lvItem.lParam;
							sNmLvDispInfo.item.iSubItem = i;

							OnMainNotify(g_window->GetHandle(), 0, (LPNMHDR)&sNmLvDispInfo);
							_stprintf(pchData, TEXT("%s : %s\n"), lvColumn.pszText, sNmLvDispInfo.item.pszText);
							uLen = _tcslen(pchData);
							pchData += uLen;
						}

						*pchData = '\n';
						_stprintf(pchData, TEXT("====================\n"));
						pchData += _tcslen(pchData);
					}				
				}

				GlobalUnlock(hClipboardData);
				SetClipboardData(CF_UNICODETEXT, hClipboardData);
			}
			
			CloseClipboard();
		}

		return true;
	}

	return false;
}

INT32 OnMainNotify(HWND hWnd, INT32 iCmdID, LPNMHDR lpNotify)
{
	if(lpNotify->hwndFrom == g_window->GetControl(IDC_LISTVIEW_AVAILPATCHES)->GetHandle())
	{
		HWND hListView = g_window->GetControl(IDC_LISTVIEW_AVAILPATCHES)->GetHandle();

		switch(lpNotify->code)
		{
		case LVN_COLUMNCLICK:
			{
				LPNMLISTVIEW lpNmLv = (LPNMLISTVIEW)lpNotify;
				WeeTools::ListView *lv = (WeeTools::ListView *)g_window->GetControl(IDC_LISTVIEW_AVAILPATCHES);

				switch(lpNmLv->iSubItem)
				{
				case 0:
					{
#ifdef HEADER_CHECKBOX
						HWND hListViewHeader;
						hListViewHeader = ListView_GetHeader(lv->GetHandle());

						HDITEM hdItem = {0};
						hdItem.mask = HDI_IMAGE;
						Header_GetItem(hListViewHeader, 0, &hdItem);
						hdItem.iImage = (hdItem.iImage == 1) ? 2 : 1;
						Header_SetItem(hListViewHeader, 0, &hdItem);

						BOOL bCheckAll = hdItem.iImage == 2 ? TRUE : FALSE;

						INT32 iItemCount = lv->GetItemCount();
						for(INT32 i = 0; i < iItemCount; i++)
						{
							ListView_SetCheckState(lv->GetHandle(), i, bCheckAll);
						}
#else
						INT32 iItemCount = lv->GetItemCount();
						for(INT32 i = 0; i < iItemCount; i++)
						{
							ListView_SetCheckState(lv->GetHandle(), i, FALSE);
						}
#endif
					}
					break;
				case 1:
					lv->Sort(ListViewCompareType);
					break;
				case 2:
					lv->Sort(ListViewCompareGroup);
					break;
				case 3:
					lv->Sort(ListViewCompareName);
					break;
				default:
					break;
				}
			}
			break;
		case LVN_ITEMCHANGING:
			{
				SetFocus(hListView);

				LPNMLISTVIEW lpNmLv = (LPNMLISTVIEW)lpNotify;

				if((lpNmLv->uNewState & LVIS_SELECTED) && ListView_GetCheckState(hListView, lpNmLv->iItem) == FALSE)
					return TRUE;

				if((lpNmLv->uOldState & LVIS_SELECTED) && ListView_GetCheckState(hListView, lpNmLv->iItem) == TRUE)
					return TRUE;

				if(lpNmLv->uNewState & LVIS_STATEIMAGEMASK && CheckActivePluginVersion(1, 1) == true)
				{
					WeePlugin::DIFFITEM *lpDiffItem = (WeePlugin::DIFFITEM *)lpNmLv->lParam;

					switch(lpNmLv->uNewState & LVIS_STATEIMAGEMASK)
					{
					case INDEXTOSTATEIMAGEMASK(BST_CHECKED + 1):
						{
							WeePlugin::NOTIFYMESSAGE sNm = {0};

							sNm.iMessage = WeePlugin::NM_ITEMPRESELECTED;
							sNm.lpData = lpDiffItem;
							
							GetActivePlugin()->lpInterface->Notify(&sNm);

							if(sNm.iRetCode != 0)
								return TRUE;
						}
						break;
					case INDEXTOSTATEIMAGEMASK(BST_UNCHECKED + 1):
						{
							WeePlugin::NOTIFYMESSAGE sNm = {0};

							sNm.iMessage = WeePlugin::NM_ITEMPREDESELECTED;
							sNm.lpData = lpDiffItem;

							GetActivePlugin()->lpInterface->Notify(&sNm);

							if(sNm.iRetCode != 0)
								return TRUE;
						}
						break;
					}
				}

			}
			return FALSE;
		case LVN_ITEMCHANGED:
			{
				LPNMLISTVIEW lpNmLv = (LPNMLISTVIEW)lpNotify;
				if((lpNmLv->uChanged & LVIF_STATE) && (lpNmLv->uNewState & LVIS_STATEIMAGEMASK))
				{				
					WeePluginManager::LPPLUGIN lpPlugin = GetActivePlugin();

					switch(lpNmLv->uNewState & LVIS_STATEIMAGEMASK)
					{
						// Selected
						case INDEXTOSTATEIMAGEMASK(BST_CHECKED + 1):
							{
								WeePlugin::DIFFITEM *lpDiffItem;
								lpDiffItem = (WeePlugin::DIFFITEM *)lpNmLv->lParam;

								WeePlugin::NOTIFYMESSAGE sNm;														

								if(lpDiffItem->lpGroup && !(CheckActivePluginVersion(1, 1) == true && lpPlugin->lpInterface->PreventGroupCollision() == false))
								{				
									INT32 iCount = ListView_GetItemCount(hListView);
									for(INT32 i = 0; i < iCount; i++)
									{
										if(i == lpNmLv->iItem)
											continue;

										LVITEM lvItem = {0};
										lvItem.mask = LVIF_PARAM;
										lvItem.iItem = i;

										ListView_GetItem(hListView, &lvItem);

										WeePlugin::DIFFITEM *lpTmpDiffItem = (WeePlugin::DIFFITEM *)lvItem.lParam;

										if(lpTmpDiffItem->lpGroup && lpTmpDiffItem->lpGroup->iGroupId == lpDiffItem->lpGroup->iGroupId)
										{
											ListView_SetCheckState(hListView, i, FALSE);
											ListView_SetItemState(hListView, i, 0, LVIS_SELECTED);
											sNm.iMessage = WeePlugin::NM_ITEMDESELECTED;
											sNm.lpData = lpTmpDiffItem;
											lpPlugin->lpInterface->Notify(&sNm);
										}
									}
								}

								sNm.iMessage = WeePlugin::NM_ITEMSELECTED;	
								sNm.lpData = lpDiffItem;
								lpPlugin->lpInterface->Notify(&sNm);
								
								ListView_SetItemState(hListView, lpNmLv->iItem, LVIS_SELECTED, LVIS_SELECTED);
							}
							break;
						// Deselected
						case INDEXTOSTATEIMAGEMASK(BST_UNCHECKED + 1):
							{
								WeePlugin::DIFFITEM *lpDiffItem;
								lpDiffItem = (WeePlugin::DIFFITEM *)lpNmLv->lParam;

								ListView_SetItemState(hListView, lpNmLv->iItem, 0, LVIS_SELECTED);

								WeePlugin::NOTIFYMESSAGE sNm;
								sNm.iMessage = WeePlugin::NM_ITEMDESELECTED;	
								sNm.lpData = lpDiffItem;
								lpPlugin->lpInterface->Notify(&sNm);
							}
							break;
					}

#ifdef HEADER_CHECKBOX
					HWND hListViewHeader;
					hListViewHeader = ListView_GetHeader(hListView);
					bool bAllChecked = true;

					//INT32 iSelCount = ListView_GetSelectedCount(hListView);

					INT32 iSelCount = 0;
					for(INT32 i = 0; i < ListView_GetItemCount(hListView); i++)
					{
						if(ListView_GetCheckState(hListView, i) == TRUE)
							iSelCount++;
					}

					WeePlugin::LPDIFFITEMLIST lpDiffList = GetActivePlugin()->lpInterface->GetDiffItems();

					INT32 iTotalCount = 0;
					for(WeePlugin::DiffItemList::const_iterator it = lpDiffList->items.begin(); it != lpDiffList->items.end(); it++)
					{
						if((*it)->lpGroup == NULL)
							iTotalCount++;
					}

					iTotalCount  += lpDiffList->groups.size();

					if(iSelCount != iTotalCount)
						bAllChecked = false;

					HDITEM hdItem = {0};
					hdItem.mask = HDI_IMAGE;
					hdItem.iImage = bAllChecked == true ? 2 : 1;
					Header_SetItem(hListViewHeader, 0, &hdItem);
#endif
				}
			}
			break;
		case LVN_GETDISPINFO:
			{
				NMLVDISPINFO *lvDisplayInfo = (NMLVDISPINFO *)lpNotify;
				if(lvDisplayInfo->hdr.idFrom == IDC_LISTVIEW_AVAILPATCHES)
				{
					WeePlugin::DIFFITEM *lpDiffItem = (WeePlugin::DIFFITEM *)lvDisplayInfo->item.lParam;

					lvDisplayInfo->item.pszText = TEXT("");

					switch(lvDisplayInfo->item.iSubItem)
					{
					case 0:
						
						break;
					case 1:
						if(lpDiffItem->lpType != NULL)
							lvDisplayInfo->item.pszText = lpDiffItem->lpType->lpszName;
						break;
					case 2:
						if(lpDiffItem->lpGroup != NULL)
							lvDisplayInfo->item.pszText = lpDiffItem->lpGroup->lpszName;
						break;
					case 3:
						if(lpDiffItem->lpszName != NULL)
							lvDisplayInfo->item.pszText = lpDiffItem->lpszName;
						break;
					case 4:
						if(lpDiffItem->lpszDescription != NULL)
							lvDisplayInfo->item.pszText = lpDiffItem->lpszDescription;
						break;
					}

					if(CheckActivePluginVersion(1, 1))
					{
						GetActivePlugin()->lpInterface->GetColumnDispInfo(lpDiffItem, lvDisplayInfo);
					}
				}
			}
			return 0;
		}
	}

	return DefWindowProc(hWnd, WM_NOTIFY, iCmdID, (LPARAM)lpNotify);
}

void OnMainPaint(HWND hWnd)
{
	RECT rc;
	GetClientRect(hWnd, &rc);

	PAINTSTRUCT ps;
	HDC hDC = BeginPaint(hWnd, &ps);
	HDC hMemDC = g_window->GetMemDC();

	FillRect(hMemDC, &rc, g_hBrush);

	RECT rcHeading;
	ZeroMemory(&rcHeading, sizeof(RECT));

	GetClientRect(hWnd, &rcHeading);

	rcHeading.left = 10;
	rcHeading.top = 10;

	rcHeading.right -= 10;
	rcHeading.bottom = 40;

	// Prepare drawing
	HFONT hOldFont = (HFONT)SelectObject(hMemDC, g_window->GetFont(1));
	SetBkMode(hMemDC, TRANSPARENT);
	COLORREF clrOld = SetTextColor(hMemDC, COLORREF(RGB(0, 70, 200)));

	// Draw diff header
	FillRect(hMemDC, &rcHeading, GetSysColorBrush(0));
	Rectangle(hMemDC, rcHeading.left, rcHeading.top, rcHeading.right, rcHeading.bottom);
	DrawText(hMemDC, g_lpszHeading, _tcslen(g_lpszHeading), &rcHeading, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

	// Draw static box descriptions
	SetTextColor(hMemDC, clrOld);
	SelectObject(hMemDC, g_window->GetFont(0));

	RECT rcWindow;
	GetClientRect(hWnd, &rcWindow);

	rcWindow.left = 10;
	rcWindow.right -= 20;
	rcWindow.top = rcWindow.bottom - 60;
	rcWindow.bottom = rcWindow.top + 20;

	DrawText(hMemDC, g_strSelectPatchEngine, _tcslen(g_strSelectPatchEngine), &rcWindow, DT_SINGLELINE);

	rcWindow.top = rcWindow.bottom - 70;
	rcWindow.bottom = rcWindow.top + 20;

	DrawText(hMemDC, g_strOutputExecutable, _tcslen(g_strOutputExecutable), &rcWindow, DT_SINGLELINE);

	rcWindow.top = rcWindow.bottom - 70;
	rcWindow.bottom = rcWindow.top + 20;

	DrawText(hMemDC, g_strSourceDiffFile, _tcslen(g_strSourceDiffFile), &rcWindow, DT_SINGLELINE);

	rcWindow.top = rcWindow.bottom - 70;
	rcWindow.bottom = rcWindow.top + 20;

	DrawText(hMemDC, g_strSourceExeFile, _tcslen(g_strSourceExeFile), &rcWindow, DT_SINGLELINE);


	SelectObject(hMemDC, hOldFont);
	BitBlt(hDC, 0, 0, rc.right, rc.bottom, hMemDC, 0, 0, SRCCOPY);
	EndPaint(hWnd, &ps);
}

void OnRefresh()
{
	WeePluginManager::LPPLUGIN lpPlugin = GetActivePlugin();
	EnableWindow(g_window->GetControl(ID_BTN_INPUT_DIFF)->GetHandle(), lpPlugin->lpInterface->NeedDiffFile());

	LPCTSTR lpszDiffTitle = lpPlugin->lpInterface->GetDiffTitle();

	if(lpszDiffTitle == NULL)
	{
		if(g_szInputDiffPath[0] != NULL)
		{
			TCHAR *p = g_szInputDiffPath + _tcslen(g_szInputDiffPath) - 1;
			while(p != g_szInputDiffPath)
			{
				if(*(p-1) == TEXT('\\'))
					break;

				p--;
			}

			_tcscpy_s(g_lpszHeading, _MAX_PATH, p);
		}
		else
		{
			_tcscpy_s(g_lpszHeading, _MAX_PATH, g_lpszNoDiffSelected);
		}
	} else
	{
		_tcscpy_s(g_lpszHeading, _MAX_PATH, lpszDiffTitle);
	}

	WeeTools::ListView *lv = (WeeTools::ListView *)g_window->GetControl(IDC_LISTVIEW_AVAILPATCHES);

	lv->Clear();

	//////////////////////////////////////////////////////////////////////////

	UINT32 uItemCount = lpPlugin->lpInterface->GetDiffCount();
	WeePlugin::LPDIFFITEMLIST lpsItems = lpPlugin->lpInterface->GetDiffItems();

	if(uItemCount > 0)
	{
		lv->SetRedraw(false);

		for(WeePlugin::DiffItemList::iterator it = lpsItems->items.begin(); it != lpsItems->items.end(); it++)
		{
			WeePlugin::DIFFITEM *lpDiffItem = (WeePlugin::DIFFITEM *)(*it);			
			UINT32 uState = lpDiffItem->uState;

			INT32 iIndex = lv->AddItem(lpDiffItem);

			if(uState & WeePlugin::ST_ENABLED)
			{
				ListView_SetCheckState(lv->GetHandle(), iIndex, TRUE);
			}
		}

		lv->Sort(ListViewCompareName);

		lv->SetRedraw(true);
	}

	InvalidateRect(g_window->GetHandle(), NULL, TRUE);
}

void OnAbout(HWND hCtrlWnd)
{
	DialogBoxParam(g_hInstance, MAKEINTRESOURCE(IDD_MESSAGEBOX), g_window->GetHandle(), AboutDlgProc, (LPARAM)NULL);
}

void OnAboutPlugin(HWND hCtrlWnd)
{
	GetActivePlugin()->lpInterface->About(g_window->GetHandle());
}

void OnInputExe(HWND hCtrlWnd)
{
	OPENFILENAME ofn = {0};

	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = g_window->GetHandle();
	ofn.hInstance = g_hInstance;
	ofn.lpstrFilter = TEXT("Ragnarok Client Executable (*.exe)\0*.exe\0\0");
	ofn.nFilterIndex = 1;
	ofn.lpstrFile = new TCHAR[_MAX_PATH];
	ZeroMemory(ofn.lpstrFile, _MAX_PATH);
	ofn.nMaxFile = _MAX_PATH;
	ofn.lpstrTitle = TEXT("Open Ragnarok Client Executable..");
	ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_ENABLEHOOK;
	ofn.lpfnHook = NULL;

	if(GetOpenFileName(&ofn))
	{
		_tcsncpy_s(g_szInputExePath, ofn.lpstrFile, _MAX_PATH);

		WeePluginManager::LPPLUGIN lpPlugin = GetActivePlugin();

		WeePlugin::NOTIFYMESSAGE sNm;
		sNm.iMessage = WeePlugin::NM_EXE_CHANGED;
		sNm.lpData = g_szInputExePath;
		lpPlugin->lpInterface->Notify(&sNm);

		Edit_SetText(g_window->GetControl(IDC_EDIT_INPUT_EXE)->GetHandle(), g_szInputExePath);

		if(Edit_GetTextLength(g_window->GetControl(IDC_EDIT_INPUT_OUT)->GetHandle()) <= 0 || (g_wgui->DisplayMessageBox(APP_TITLE, TEXT("Adjust output executable?"), NULL, MBI_QUESTION, MB_YESNO) & MB_TYPEMASK) == IDYES)
		{
			_stprintf_s(g_szOutputExePath, TEXT("%s.patched.exe"), g_szInputExePath);
			Edit_SetText(g_window->GetControl(IDC_EDIT_INPUT_OUT)->GetHandle(), g_szOutputExePath);
			sNm.iMessage = WeePlugin::NM_OUTPUT_CHANGED;
			sNm.lpData = g_szOutputExePath;
			lpPlugin->lpInterface->Notify(&sNm);
		}

		OnRefresh();
	}

	delete[] ofn.lpstrFile;
}

void OnInputDiff(HWND hCtrlWnd)
{
	OPENFILENAME ofn = {0};

	TCHAR szFilter[_MAX_PATH];

	_tcsncpy_s(szFilter, TEXT("Supported diff files("), _MAX_PATH);
	TCHAR *p = szFilter + _tcslen(szFilter);

	FillSupportedDiffs(&p);
	*(p++) = TEXT(')');
	*(p++) = TEXT('\0');

	if(_tcslen(szFilter) * 2 < _MAX_PATH)
	{
		FillSupportedDiffs(&p);
		*(p++) = TEXT('\0');
	}

	*(p++) = TEXT('\0');

	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = g_window->GetHandle();
	ofn.hInstance = g_hInstance;
	ofn.lpstrFilter = szFilter;
	ofn.nFilterIndex = 1;
	ofn.lpstrFile = new TCHAR[_MAX_PATH];
	ZeroMemory(ofn.lpstrFile, _MAX_PATH);
	ofn.nMaxFile = _MAX_PATH;
	ofn.lpstrTitle = TEXT("Open Diff file..");
	ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_ENABLEHOOK;
	ofn.lpfnHook = NULL;

	if(GetOpenFileName(&ofn))
	{
		bool bFoundPlugin = false;
		_tcsncpy_s(g_szInputDiffPath, ofn.lpstrFile, _MAX_PATH);

		WeePlugin::NOTIFYMESSAGE sNm;
		sNm.iMessage = WeePlugin::NM_DIFF_CHANGED;
		sNm.lpData = g_szInputDiffPath;

		WeePluginManager::LPPLUGIN lpOldPlugin = GetActivePlugin();

		for(INT32 i = 0; i < ComboBox_GetCount(g_window->GetControl(IDC_COMBOBOX_PLUGIN)->GetHandle()); i++)
		{
			WeePluginManager::LPPLUGIN lpPlugin = (WeePluginManager::LPPLUGIN)ComboBox_GetItemData(g_window->GetControl(IDC_COMBOBOX_PLUGIN)->GetHandle(), i);
			
			TCHAR *ext;
			LPCTSTR lpszRef = lpPlugin->lpInterface->GetSupportedFormat();
			UINT32 uCharIndex = _tcslen(g_szInputDiffPath);

			while(uCharIndex-- > 0)
			{
				if(g_szInputDiffPath[uCharIndex] == TEXT('.'))
				{
					ext = g_szInputDiffPath + uCharIndex;
					break;
				}
			}

			if(_tcsicmp(ext, lpszRef) == 0)
			{
				ComboBox_SetCurSel(g_window->GetControl(IDC_COMBOBOX_PLUGIN)->GetHandle(), i);
				bFoundPlugin = true;
				break;
			}
		}

		if(bFoundPlugin == false)
		{
			DisplayMessageBox(APP_TITLE, TEXT("No plug-in found that supports the selected file format!"), NULL, MBI_WARNING, MB_OK);
		}
		else
		{
			GetActivePlugin()->lpInterface->Notify(&sNm);
			Edit_SetText(g_window->GetControl(IDC_EDIT_INPUT_DIFF)->GetHandle(), g_szInputDiffPath);
			OnRefresh();
		}
	}

	delete[] ofn.lpstrFile;
}

void OnOutputExe(HWND hCtrlWnd)
{
	OPENFILENAME ofn = {0};

	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = g_window->GetHandle();
	ofn.hInstance = g_hInstance;
	ofn.lpstrFilter = TEXT("Ragnarok Client Executable (*.exe)\0*.exe\0\0");
	ofn.nFilterIndex = 1;
	ofn.lpstrFile = new TCHAR[_MAX_PATH];
	ZeroMemory(ofn.lpstrFile, _MAX_PATH);
	ofn.nMaxFile = _MAX_PATH;
	ofn.lpstrTitle = TEXT("Select save path..");
	ofn.Flags = OFN_EXPLORER | OFN_ENABLEHOOK;
	ofn.lpfnHook = NULL;

	if(GetOpenFileName(&ofn))
	{		
		TCHAR *p = ofn.lpstrFile + _tcslen(ofn.lpstrFile) - 4;
		
		if(_tcsicmp(p, TEXT(".exe")) != 0)
		{					
			_stprintf_s(g_szOutputExePath, _MAX_PATH, TEXT("%s.exe"), ofn.lpstrFile);
		} else
		{
			p = ofn.lpstrFile + ofn.nFileOffset;

			if(_tcslen(p) == 4)
			{
				*p = TEXT('\0');
				_stprintf_s(g_szOutputExePath, _MAX_PATH, TEXT("%sempty.exe"), ofn.lpstrFile);
			}
			else
				_tcsncpy_s(g_szOutputExePath, ofn.lpstrFile, _MAX_PATH);
		}

		WeePlugin::NOTIFYMESSAGE sNm;
		sNm.iMessage = WeePlugin::NM_OUTPUT_CHANGED;
		sNm.lpData = g_szOutputExePath;

		GetActivePlugin()->lpInterface->Notify(&sNm);
		
		Edit_SetText(g_window->GetControl(IDC_EDIT_INPUT_OUT)->GetHandle(), g_szOutputExePath);
	}

	delete[] ofn.lpstrFile;
}

void OnPatchIt(HWND hCtrlWnd)
{
	HWND hListView = g_window->GetControl(IDC_LISTVIEW_AVAILPATCHES)->GetHandle();
	LVITEM lvItem = {0};
	lvItem.mask = LVIF_PARAM;

	for(INT32 i = 0; i < ListView_GetItemCount(hListView); i++)
	{
		lvItem.iItem = i;

		ListView_GetItem(hListView, &lvItem);

		WeePlugin::DIFFITEM *lpDiffItem = (WeePlugin::DIFFITEM *)lvItem.lParam;

		if((lpDiffItem->uState & WeePlugin::ST_ENABLED) && ListView_GetCheckState(hListView, i) == FALSE)
		{
			lpDiffItem->uState ^= WeePlugin::ST_ENABLED;
		} else if ((lpDiffItem->uState & WeePlugin::ST_ENABLED) == 0 && ListView_GetCheckState(hListView, i) == TRUE)
		{
			lpDiffItem->uState |= WeePlugin::ST_ENABLED;
		}
	}

	Edit_GetText(g_window->GetControl(IDC_EDIT_INPUT_OUT)->GetHandle(), g_szOutputExePath, _MAX_PATH);

	WeePlugin::NOTIFYMESSAGE sNm;
	sNm.iMessage = WeePlugin::NM_OUTPUT_CHANGED;
	sNm.lpData = g_szOutputExePath;

	GetActivePlugin()->lpInterface->Notify(&sNm);

	GetActivePlugin()->lpInterface->PatchIt();
}

void OnMainCommand(HWND hWnd, INT32 iCtrlID, HWND hCtrlWnd)
{
	if(LOWORD(iCtrlID) == IDC_COMBOBOX_PLUGIN)
	{
		if(HIWORD(iCtrlID) == CBN_SELCHANGE)
		{
			WeeTools::ListView *lv = (WeeTools::ListView *)g_window->GetControl(IDC_LISTVIEW_AVAILPATCHES);

			if(g_uCustomColumns > 0)
			{
				for(UINT32 i = 0; i < g_uCustomColumns; i++)
				{
					ListView_DeleteColumn(lv->GetHandle(), 5);
				}
			}

			g_uCustomColumns = 0;

			WeePlugin::NOTIFYMESSAGE sNm;
			WeePluginManager::LPPLUGIN lpPlugin = GetActivePlugin();

			if(CheckActivePluginVersion(1, 1))
			{
				WeePlugin::ColumnNames *ColNames = lpPlugin->lpInterface->GetColumnNames();

				if(ColNames->size() > 0)
				{
					for(WeePlugin::ColumnNames::iterator it = ColNames->begin(); it != ColNames->end(); it++)
					{
						lv->AddHeader(5+g_uCustomColumns, LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM, LVCFMT_LEFT, 80, *it);
						g_uCustomColumns++;
					}
				}
			}
			
			sNm.iMessage = WeePlugin::NM_EXE_CHANGED;
			sNm.lpData = g_szInputExePath;
			lpPlugin->lpInterface->Notify(&sNm);

			sNm.iMessage = WeePlugin::NM_DIFF_CHANGED;
			sNm.lpData = g_szInputDiffPath;
			lpPlugin->lpInterface->Notify(&sNm);

			sNm.iMessage = WeePlugin::NM_OUTPUT_CHANGED;
			sNm.lpData = g_szOutputExePath;
			lpPlugin->lpInterface->Notify(&sNm);			

			OnRefresh();
		}
	}

#define REDIRECT(name, fn) case ID_##name: ##fn(hCtrlWnd); break
	switch(iCtrlID)
	{
		REDIRECT(ABOUT,				OnAbout);
		REDIRECT(ABOUT_PLUGIN,		OnAboutPlugin);
		REDIRECT(BTN_INPUT_EXE,		OnInputExe);
		REDIRECT(BTN_INPUT_DIFF,	OnInputDiff);
		REDIRECT(BTN_INPUT_OUT,		OnOutputExe);
		REDIRECT(BTN_PATCHIT,		OnPatchIt);
	}
#undef REDIRECT
}

void Cleanup()
{
	if(g_wgui != NULL)
	{
		delete g_wgui;
		g_wgui = NULL;
	}

	if(g_wpm != NULL)
	{
		delete g_wpm;
		g_wpm = NULL;
	}

	if(g_window != NULL)
	{
		delete g_window;
		g_window = NULL;
	}
}

void QuitWithError(LPCTSTR errorMsg)
{
	HWND hParent = NULL;

	if (g_window != NULL)
		hParent = g_window->GetHandle();

	DisplayMessageBox(TEXT("Runtime Error!"), errorMsg, NULL, MBI_ERROR, MB_OK);
	Cleanup();

	exit(EXIT_FAILURE);
}

LRESULT CALLBACK SubClassListViewProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_KEYDOWN:
			if(OnKeydown(hWnd, wParam))
				return true;

			break;
	}

	return CallWindowProc(g_OldListViewProc, hWnd, uMsg, wParam, lParam);
}

INT_PTR CALLBACK AboutDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static HWND hButton = NULL;
	static HBRUSH hBrush = CreateSolidBrush(RGB(0xFF, 0xFF, 0xFF));
	static HIMAGELIST hImageList = ImageList_Create(48, 48, ILC_COLOR32, 0, IDIL_MAX);
	static SIZE sizeText;

	TCHAR szAboutMsg[] =
		TEXT("This tool is part of the WeeTools collection.\r\n") \
		TEXT("This software is provided AS IS and without ANY expressed or implied warranties, including but not limited to the implied warranties of merchantability and fitness for a particular purpose.\r\n") \
		TEXT("You are using this software on your own. The author shall not be held liable for ANY damage resulting from the use of this software, either directly or indirectly, including but not limited to loss of data.\r\n\r\n") \
		TEXT("WeeTools © ") APP_YEAR TEXT(" Shinryo (shinryo@hotmail.de)\r\n") \
		TEXT("All rights reserved.\r\n\r\n") \
		TEXT("Thanks to:\r\n") \
		TEXT("Maldiablo - for his first diff patcher\r\n") \
		TEXT("k3dt -another nice diff patcher\r\n") \
		TEXT("www.aha-soft.com - free icons\r\n\r\n") \
		TEXT("Diff contributors:\r\n") \
		TEXT("Skotlex, mrmagoo, Rasqual, glucose, Zephiris, Rodney Jason, Meruru, -o-, Diablo, lord chris, flaviojs, Brainstorm, Yommy, Adrilindozao, Fabio, Myst, Sirius_White, LightFighter, Waeyan, Ai4rei");

	switch(uMsg) 
	{
	case WM_INITDIALOG:
		{
			HDC hDC = GetDC(hWnd);
			SetWindowText(hWnd, TEXT("About"));

			int nButtonWidth = MulDiv(38, LOWORD(GetDialogBaseUnits()), 4);
			int nButtonHeight = MulDiv(12, LOWORD(GetDialogBaseUnits()), 4);

			SetWindowPos(hWnd, NULL, 0, 0, 78 + 370 + 10, 20 + 15 * MulDiv(14, LOWORD(GetDialogBaseUnits()), 4) + 10, SWP_NOMOVE);

			RECT rcClient;
			GetClientRect(hWnd, &rcClient);

			int x = rcClient.right - 5 - nButtonWidth;
			int y = rcClient.bottom - 5 - nButtonHeight;

			HWND hButton = CreateWindowEx(0, WC_BUTTON, TEXT("OK"), WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, x, y, nButtonWidth, nButtonHeight, hWnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(IDOK)), g_hInstance, NULL);
			SendMessage(hButton, WM_SETFONT, (WPARAM)g_window->GetFont(0), true);
			SelectObject(hDC, g_window->GetFont(0));

			WeeTools::Window::Center(hWnd);
		}
		return true;
	case WM_PAINT:
		{
			RECT rcClient;
			GetClientRect(hWnd, &rcClient);

			PAINTSTRUCT ps;
			HDC hDC = BeginPaint(hWnd, &ps);

			SelectObject(hDC, g_window->GetFont(0));

			rcClient.bottom -= MulDiv(12, LOWORD(GetDialogBaseUnits()), 4) + 10;

			FillRect(hDC, &rcClient, hBrush);

			rcClient.left += 78;
			rcClient.top += 20;
			rcClient.right -= 20;

			LOGFONT lf;
			GetObject(g_window->GetFont(0), sizeof(LOGFONT), &lf);
			lf.lfWeight = FW_BOLD;

			HFONT hMemFontBold = CreateFontIndirect(&lf);
			SelectObject(hDC, hMemFontBold);
			DrawText(hDC, APP_TITLE TEXT(" ") APP_VERSION, -1, &rcClient, DT_SINGLELINE);
			DeleteObject(hMemFontBold);
			SelectObject(hDC, g_window->GetFont(0));

			rcClient.top += 30;

			DrawText(hDC, szAboutMsg, -1, &rcClient, DT_TABSTOP | DT_WORDBREAK);

			HBITMAP hBmp = (HBITMAP)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_WEETOOLS), IMAGE_BITMAP, 0, 0, 0);
			HDC hMemDC = CreateCompatibleDC(hDC);
			SelectObject(hMemDC, hBmp);

			BitBlt(hDC, 20, 20, 48, 48, hMemDC, 0, 0, SRCCOPY);

			DeleteDC(hMemDC);
			DeleteObject(hBmp);

			EndPaint(hWnd, &ps);
		}
		return false;
	case WM_COMMAND:
	case WM_CLOSE:
		DeleteObject(hBrush);
		EndDialog(hWnd, 0);
		return true;
	}

	return false;
}

LRESULT CALLBACK DoubleBufferProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	WNDPROC origProc = g_window->GetControl(GetDlgCtrlID(hWnd))->GetOriginalWndProc();

	switch(uMsg)
	{
	case WM_PAINT:
		{
			HDC hDC = GetDC(hWnd);
			HDC hMemDC = g_window->GetMemDC();
			RECT rc;
			GetClientRect(hWnd, &rc);

			RECT rcGlobal = {0};
			rcGlobal.right = GetSystemMetrics(SM_CXSCREEN);
			rcGlobal.bottom = GetSystemMetrics(SM_CYSCREEN);

			FillRect(hMemDC, &rcGlobal, g_hBrush);

			CallWindowProc(origProc, hWnd, WM_PRINT, (WPARAM)hMemDC, PRF_ERASEBKGND | PRF_CHILDREN | PRF_CLIENT | PRF_OWNED);			

			BitBlt(hDC, 0, 0, rc.right, rc.bottom, hMemDC, 0, 0, SRCCOPY);

			ReleaseDC(hWnd, hDC);
			ValidateRect(hWnd, NULL);
		}
		return 0;
	case WM_ERASEBKGND:
		return TRUE;
	}

	return CallWindowProc(origProc, hWnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK WndProcMain(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{	
	switch(uMsg) {
	case WM_NOTIFY:
		return OnMainNotify(hWnd, wParam, (LPNMHDR)lParam);
	case WM_COMMAND:
		try 
		{
			OnMainCommand(hWnd, wParam, reinterpret_cast<HWND>(lParam));
		} catch (WeeException &e)
		{
			DisplayException(e.GetMessage(), e.GetType());
		}
		break;
	case WM_GETMINMAXINFO:
		{
			LPMINMAXINFO lpMinMaxInfo = (LPMINMAXINFO)lParam;
			lpMinMaxInfo->ptMinTrackSize.x = 320;
			lpMinMaxInfo->ptMinTrackSize.y = 360;
		}
		return 0;
	case WM_SIZE:	
		OnMainSize(hWnd, LOWORD(lParam), HIWORD(lParam));
		return 0;
	case WM_ERASEBKGND:
		return TRUE;
	case WM_PAINT:
		OnMainPaint(hWnd);
		return 0;
		return 0;
	case WM_KEYDOWN:
		OnKeydown(hWnd, wParam);
		return 0;
	case WM_CREATE:
		OnMainCreate(hWnd, (CREATESTRUCT*)lParam);
		return 0;
	case WM_CLOSE:
		DestroyWindow(hWnd);
		return 0;
	case WM_DESTROY:
		OnMainDestroy(hWnd);			
		PostQuitMessage(EXIT_SUCCESS);
		return 0;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );

	int iExitCode = 0;

	try
	{
		g_hInstance = hInstance;

		GetModuleFileName(hInstance, g_szPatcherPath, _MAX_PATH);

		UINT32 i = _tcslen(g_szPatcherPath);

		while(g_szPatcherPath[i] != TEXT('\\'))
		{
			if(i-- == 0)
				break;
		}

		if(i != 0)
		{
			g_szPatcherPath[i+1] = TEXT('\0');
		}

		g_window = new WeeTools::Window(WndProcMain, APP_TITLE, 0, 0, APP_WIDTH, APP_HEIGHT, 0, WS_CLIPCHILDREN | WS_CLIPSIBLINGS, 0);
		g_window->UseCommonControls(ICC_BAR_CLASSES);

		g_window->AddFont(1, CreateFont(25, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, TEXT("Arial")));

		INITCONTROL sInitCtrl;
		ZeroMemory(&sInitCtrl, sizeof(sInitCtrl));

		sInitCtrl.hInstance = hInstance;
		sInitCtrl.hParent = g_window->GetHandle();

		g_wpm = new WeePluginManager();
		g_wgui = new WeeGUI(hInstance, g_window->GetHandle());

		//////////////////////////////////////////////////////////////////////////
		// ListView

		sInitCtrl.lpszClassName = WC_LISTVIEW;
		sInitCtrl.id = IDC_LISTVIEW_AVAILPATCHES;
		sInitCtrl.dwStyle = LVS_REPORT | LVS_SHOWSELALWAYS | WS_BORDER;		

		WeeTools::ListView *lv = new WeeTools::ListView(&sInitCtrl);
		lv->SetStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_DOUBLEBUFFER | LVS_EX_CHECKBOXES);
		g_window->AddControl(lv);

		lv->AddHeader(0, LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM, LVCFMT_LEFT, 27, TEXT(""));
		lv->AddHeader(1, LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM, LVCFMT_LEFT, 50, TEXT("Type"));
		lv->AddHeader(2, LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM, LVCFMT_LEFT, 70, TEXT("Group"));
		lv->AddHeader(3, LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM, LVCFMT_LEFT, 250, TEXT("Name"));
		lv->AddHeader(4, LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM, LVCFMT_LEFT, 200, TEXT("Description"));

		g_OldListViewProc = (WNDPROC)SetWindowLong(lv->GetHandle(), GWL_WNDPROC, (LONG)SubClassListViewProc);

#ifdef HEADER_CHECKBOX
		HWND hListViewHeader = ListView_GetHeader(lv->GetHandle());

		HBITMAP hBmp;
		HIMAGELIST hImageList = ImageList_Create(16, 16, ILC_COLOR32, 0, 3);
		hBmp = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_CHECKBOXES), IMAGE_BITMAP, 0, 0, 0);
		ImageList_Add(hImageList, hBmp, NULL);
		DeleteObject(hBmp);

		Header_SetImageList(hListViewHeader, hImageList);

		HDITEM hdItem = {0};
		hdItem.mask = HDI_IMAGE | HDI_FORMAT;
		Header_GetItem(hListViewHeader, 0, &hdItem);
		hdItem.iImage = 1;
		hdItem.fmt |= HDF_IMAGE;
		Header_SetItem(hListViewHeader, 0, &hdItem);
#endif

		//////////////////////////////////////////////////////////////////////////
		// Plugin combobox

		sInitCtrl.procDoubleBuffer = NULL;
		sInitCtrl.lpszClassName = WC_COMBOBOX;
		sInitCtrl.id = IDC_COMBOBOX_PLUGIN;
		sInitCtrl.dwStyle = CBS_DROPDOWNLIST | CBS_SORT;
		sInitCtrl.dwExStyle = 0;

		g_window->AddControl(new WeeTools::Control(&sInitCtrl));

		//////////////////////////////////////////////////////////////////////////
		// Input boxes

		//sInitCtrl.procDoubleBuffer = DoubleBufferProc;

		sInitCtrl.lpszClassName = WC_EDIT;
		sInitCtrl.id = IDC_EDIT_INPUT_OUT;
		sInitCtrl.lpszCaption = g_szOutputExePath;
		sInitCtrl.dwStyle = WS_BORDER;
		sInitCtrl.dwExStyle = 0;

		g_window->AddControl(new WeeTools::Control(&sInitCtrl));

		sInitCtrl.id = IDC_EDIT_INPUT_DIFF;
		sInitCtrl.lpszCaption = g_szInputDiffPath;
		g_window->AddControl(new WeeTools::Control(&sInitCtrl));

		sInitCtrl.id = IDC_EDIT_INPUT_EXE;
		sInitCtrl.lpszCaption = g_szInputExePath;
		g_window->AddControl(new WeeTools::Control(&sInitCtrl));

		EnableWindow(g_window->GetControl(IDC_EDIT_INPUT_DIFF)->GetHandle(), FALSE);
		EnableWindow(g_window->GetControl(IDC_EDIT_INPUT_EXE)->GetHandle(), FALSE);

		//////////////////////////////////////////////////////////////////////////
		// Buttons

		sInitCtrl.lpszClassName = WC_BUTTON;
		sInitCtrl.id = ID_BTN_INPUT_EXE;
		sInitCtrl.lpszCaption = TEXT("Select..");
		sInitCtrl.dwStyle = 0;
		sInitCtrl.dwExStyle = 0;
		g_window->AddControl(new WeeTools::Control(&sInitCtrl));

		sInitCtrl.lpszClassName = WC_BUTTON;
		sInitCtrl.id = ID_BTN_INPUT_DIFF;
		g_window->AddControl(new WeeTools::Control(&sInitCtrl));

		sInitCtrl.lpszClassName = WC_BUTTON;
		sInitCtrl.id = ID_BTN_INPUT_OUT;
		g_window->AddControl(new WeeTools::Control(&sInitCtrl));

		sInitCtrl.lpszClassName = WC_BUTTON;
		sInitCtrl.id = ID_BTN_PATCHIT;
		sInitCtrl.lpszCaption = TEXT("Patch it!");
		g_window->AddControl(new WeeTools::Control(&sInitCtrl));

		sInitCtrl.lpszClassName = WC_BUTTON;
		sInitCtrl.id = ID_ABOUT;
		sInitCtrl.lpszCaption = TEXT("About");
		g_window->AddControl(new WeeTools::Control(&sInitCtrl));

		sInitCtrl.lpszClassName = WC_BUTTON;
		sInitCtrl.id = ID_ABOUT_PLUGIN;
		sInitCtrl.lpszCaption = TEXT("Plugin Info");
		g_window->AddControl(new WeeTools::Control(&sInitCtrl));

		//////////////////////////////////////////////////////////////////////////
		// Init plugins

		TCHAR szPluginPath[_MAX_PATH];
		_stprintf_s(szPluginPath, TEXT("%splugins\\"), g_szPatcherPath);
		
		if(InitPlugins(szPluginPath) < 0)
			throw new WeeException(TEXT("You have to place at least one plug-in inside the plug-ins folder!"), WeeException::E_ERROR);

		// Set default heading
		_tcscpy_s(g_lpszHeading, _MAX_PATH, g_lpszNoDiffSelected);

		SendMessage(g_window->GetHandle(), WM_COMMAND, MAKELONG(IDC_COMBOBOX_PLUGIN, CBN_SELCHANGE), (LPARAM)GetDlgItem(g_window->GetHandle(), IDC_COMBOBOX_PLUGIN));

		g_window->Show();
		iExitCode = g_window->MessageLoop();
	}
	catch (WeeException *e)
	{
		QuitWithError(e->GetMessage());
	}
	catch (std::exception &e)
	{
		MessageBoxA(NULL, e.what(), "Runtime Error", MB_OK);
	}

	Cleanup();

	//_CrtDumpMemoryLeaks();

	return iExitCode;
}