#ifndef _LISTVIEW_H
#define _LISTVIEW_H

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

#include <Windows.h>
#include "Control.h"

namespace WeeTools
{

class ListView : public Control
{
public:
	ListView(LPINITCONTROL lpInitControl) : Control(lpInitControl)
	{
	}

	void SetStyle(DWORD dwStyle) 
	{ 
		ListView_SetExtendedListViewStyle(m_hWnd, dwStyle); 
	}

	void AddHeader(INT32 iIndex, UINT32 uMask, INT32 iFmt, INT32 iWidth, LPTSTR lpszCaption)
	{
		LV_COLUMN lvColum = {0};
		lvColum.mask = uMask;
		lvColum.fmt = iFmt;
		lvColum.cx = iWidth;
		lvColum.pszText = lpszCaption;
		lvColum.iSubItem = iIndex;

		ListView_InsertColumn(m_hWnd, iIndex, &lvColum);
	}

	void DeleteHeader(INT32 iIndex)
	{
		ListView_DeleteColumn(m_hWnd, iIndex);
	}

	UINT32 AddItem(LPVOID itemData) 
	{ 
		LV_ITEM lvItem = {0};
		lvItem.mask = LVIF_TEXT | LVIF_PARAM;
		lvItem.pszText = LPSTR_TEXTCALLBACK;
		lvItem.lParam = (LPARAM)itemData;

		return ListView_InsertItem(m_hWnd, &lvItem);
	}

	UINT32 DelItem(LPVOID itemData) 
	{ 
		LVFINDINFO lvFindInfo = {0};
		lvFindInfo.flags = LVFI_PARAM;
		lvFindInfo.lParam = (LPARAM)itemData;

		INT32 iIndex = ListView_FindItem(m_hWnd, -1, &lvFindInfo);
		if(iIndex == -1)
			return -1;

		return ListView_DeleteItem(m_hWnd, iIndex);
	}

	INT32 GetItemCount() 
	{ 
		return ListView_GetItemCount(m_hWnd); 
	}

	UINT32 GetSelCount() 
	{ 
		return ListView_GetSelectedCount(m_hWnd); 
	}

	void Clear() 
	{ 
		ListView_DeleteAllItems(m_hWnd); 
	}

	void SelectAll() 
	{ 
		if(GetItemCount() > 0)
			ListView_SetItemState(m_hWnd, -1, LVIS_SELECTED, LVIS_SELECTED);
	}

	void DeselectAll()
	{
		if(GetItemCount() > 0)
			ListView_SetItemState(m_hWnd, -1, 0, LVIS_SELECTED);
	}

	void SelectItem(INT32 iIndex)
	{
		if(iIndex < 0 || iIndex >= GetItemCount())
			return;

		ListView_SetItemState(m_hWnd, iIndex, LVIS_SELECTED, LVIS_SELECTED);
	}

	void SetRedraw(bool bEnabled) 
	{ 
		SendMessage(m_hWnd, WM_SETREDRAW, (WPARAM)bEnabled, 0); 
		::SetFocus(m_hWnd); 
	}

	void SetFocus(INT32 iIndex)
	{
		ListView_SetItemState(m_hWnd, -1, 0, LVIS_SELECTED);
		ListView_SetItemState(m_hWnd, iIndex, LVIS_SELECTED, LVIS_SELECTED);
		ListView_EnsureVisible(m_hWnd, iIndex, false);
	}

	void Sort(PFNLVCOMPARE(_pfnCompare), INT32 iColumIndex = 0) 
	{ 
		ListView_SortItems(m_hWnd, _pfnCompare, iColumIndex); 
	}

	LPVOID GetFirstSelected()
	{
		INT32 iItemCount = GetItemCount();
		INT32 iSelCount = GetSelCount();

		LVITEM lvItem = {0};

		if(iSelCount > 0)
		{
			lvItem.mask = LVIF_STATE | LVIF_PARAM;
			lvItem.stateMask = LVIS_SELECTED;

			INT32 i;

			for(i = 0; i < iItemCount; i++)
			{
				lvItem.iItem = i;
				ListView_GetItem(m_hWnd, &lvItem);

				if(lvItem.state == LVIS_SELECTED)
					break;
			}

			if(i == iItemCount)
				return NULL;

			return (LPVOID)lvItem.lParam;
		}

		return NULL;
	}
};

}

#endif // _LISTVIEW_H
