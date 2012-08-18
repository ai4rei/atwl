#include "WeeMessageBox.h"

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

#include "Window.h"

#define IDC_MSGDLG_STR_BG RGB(255, 255, 255)
#define WMB_ICON_SIZE 32
#define WMB_ICON_POSX 50
#define WMB_FONT_SIZE 11
#define PADDING_LEFT  WMB_ICON_POSX + WMB_ICON_SIZE + 10;

LPMSGBOXINFO g_mbi = NULL;
HIMAGELIST g_hImageList = NULL;
HMODULE g_user32lib = NULL;
INT32 g_nFirstButton = -1;

HFONT g_hOldFont = NULL, g_hFont = NULL;
HDC g_hDC = NULL;

bool IsWindowsVistaOrLater()
{
	OSVERSIONINFO osvi;

	ZeroMemory(&osvi, sizeof(osvi));
	osvi.dwOSVersionInfoSize = sizeof(osvi);

	GetVersionEx(&osvi);

	return osvi.dwMajorVersion >= 6;
}

inline INT32 GetButtonWidth()
{
	return MulDiv(38, LOWORD(GetDialogBaseUnits()), 4);
}

inline INT32 GetButtonHeight()
{
	return MulDiv(12, HIWORD(GetDialogBaseUnits()), 8);
}

HWND AddButton(HWND hWnd, HINSTANCE hInstance, INT32 iIDDlgItem, INT32 iCaptionID, HWND hLastButton = NULL, bool bIsDefault = false)
{
	TCHAR str[_MAX_PATH];

	if(g_user32lib == NULL)
		return NULL;

	LoadString(g_user32lib, iCaptionID, str, _MAX_PATH);

	int iButtonWidth = GetButtonWidth();
	int iButtonHeight = GetButtonHeight();

	RECT rcWindow, rcLastButton;
	SIZE sizeWindow;

	GetClientRect(hWnd, &rcWindow);

	sizeWindow.cx = rcWindow.right;
	sizeWindow.cy = rcWindow.bottom;

	SIZE sizeText;

	GetTextExtentPoint(g_hDC, str, _tcslen(str), &sizeText);
	if(sizeText.cx > iButtonWidth)
		iButtonWidth = sizeText.cx + 10;

	int x = iButtonWidth + 10;
	int y = 70 + iButtonHeight + 20;

	if(hLastButton != NULL) 
	{
		GetClientRect(hLastButton, &rcLastButton);
		MapWindowPoints(hLastButton, hWnd, (LPPOINT)&rcLastButton, 2);
		x = rcLastButton.left - x;
	} else {
		x = sizeWindow.cx - x;
	}

	HWND hButton = CreateWindowEx(0, TEXT("button"), str, WS_CHILD | WS_VISIBLE | ( bIsDefault ? BS_DEFPUSHBUTTON : 0), x, y, iButtonWidth, iButtonHeight, hWnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(iIDDlgItem)), hInstance, NULL);
	
	if(bIsDefault) 
	{
		SetFocus(hButton);
		SendMessage(hWnd, DM_SETDEFID, iIDDlgItem, 0);
	}

	SendMessage(hButton, WM_SETFONT, (WPARAM)g_hFont, true);

	return hButton;
}

void MoveControl(HWND hParent, INT iIDDlgItem, DWORD32 x, DWORD32 y)
{
	RECT rcControl;

	if(hParent == NULL)
		return;

	HWND hControl = GetDlgItem(hParent, iIDDlgItem);

	if(hControl == NULL)
		return;

	GetClientRect(hControl, &rcControl);
	MapWindowPoints(hControl, hParent, (LPPOINT)&rcControl, 2);

	SetWindowPos(hControl, NULL, rcControl.left+x, rcControl.top+y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
}

void OnCreateDlg(HWND hWnd)
{
	if(g_mbi != NULL) {
		g_hDC = CreateCompatibleDC(GetDC(hWnd));
		g_user32lib = LoadLibrary(TEXT("User32.dll"));

		SetWindowText(hWnd, g_mbi->lpCaption);

		if(g_hImageList == NULL) 
		{
			HBITMAP hBmp;
			g_hImageList = ImageList_Create(WMB_ICON_SIZE, WMB_ICON_SIZE, ILC_COLOR32, 0, MBI_MAX);
			hBmp = (HBITMAP)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(g_mbi->uIconResourceID), IMAGE_BITMAP, 0, 0, 0);
			ImageList_Add(g_hImageList, hBmp, NULL);
			DeleteObject(hBmp);
		}

		// ******************************************************
		// Select font into device context for this session

		NONCLIENTMETRICS ncm;
		ncm.cbSize = sizeof(NONCLIENTMETRICS) - (IsWindowsVistaOrLater() ? 0 : 4);
		SystemParametersInfo(SPI_GETNONCLIENTMETRICS, ncm.cbSize, &ncm, 0);

		g_hFont = CreateFontIndirect(&ncm.lfMessageFont);
		g_hOldFont = (HFONT)SelectObject(g_hDC, g_hFont);

		// ******************************************************
		// Get text area size

		SIZE sizeText, sizeWindow;
		RECT rcWindow;
		GetTextExtentPoint(g_hDC, g_mbi->lpText, _tcslen(g_mbi->lpText), &sizeText);

		GetWindowRect(hWnd, &rcWindow);
		sizeWindow.cx = rcWindow.right - rcWindow.left;
		sizeWindow.cy = rcWindow.bottom - rcWindow.top;

		sizeText.cx += PADDING_LEFT;

		// Resize if necessary
		if(sizeWindow.cx < sizeText.cx + WMB_ICON_POSX + 10) 
			sizeWindow.cx += sizeText.cx - sizeWindow.cx + WMB_ICON_POSX + 10;

		POINT pCenter = WeeTools::Window::GetCenterPoint(sizeWindow);

		// ******************************************************
		// Create check box and align window size

		SIZE sizeCheckbox;

		if(g_mbi->lpCheckboxText == NULL)
			g_mbi->lpCheckboxText = TEXT("<none>");

		GetTextExtentPoint(g_hDC, g_mbi->lpCheckboxText, _tcslen(g_mbi->lpCheckboxText), &sizeCheckbox);
		sizeCheckbox.cx += 50;

		if(g_mbi->iStyle & MB_CHECKBOX) 
		{
			RECT rcDlg;
			HWND hCheckbox = CreateWindowEx(0, TEXT("button"), g_mbi->lpCheckboxText, WS_CHILD | WS_VISIBLE | BS_CHECKBOX | BS_AUTOCHECKBOX, 0, 0, 0, 0, hWnd, (HMENU)IDC_CHECKBOX, g_mbi->hInstance, NULL);
			SendMessage(hCheckbox, WM_SETFONT, (WPARAM)g_hFont, true);

			SetWindowPos(hWnd, NULL, pCenter.x, pCenter.y, sizeWindow.cx, sizeWindow.cy, SWP_NOZORDER);

			GetClientRect(hWnd, &rcDlg);			
			SetWindowPos(hCheckbox, NULL, 10, rcDlg.bottom - sizeCheckbox.cy - 10, sizeCheckbox.cx, sizeCheckbox.cy, 0);
		} else {
			SetWindowPos(hWnd, NULL, pCenter.x, pCenter.y, sizeWindow.cx, sizeWindow.cy - sizeCheckbox.cy - 20, SWP_NOZORDER);
		}

		// ******************************************************
		// Create buttons

		HWND hButton = NULL;

		switch(g_mbi->iStyle & MB_TYPEMASK)
		{
			case MB_YESNOCANCEL:
				hButton = AddButton(hWnd, g_mbi->hInstance, IDCANCEL, CANCEL_CAPTION, hButton);
			case MB_YESNO:
				hButton = AddButton(hWnd, g_mbi->hInstance, IDNO, NO_CAPTION, hButton);
				hButton = AddButton(hWnd, g_mbi->hInstance, IDYES, YES_CAPTION, hButton, true);
				g_nFirstButton = IDYES;
			break;
			case MB_OKCANCEL:
				hButton = AddButton(hWnd, g_mbi->hInstance, IDCANCEL, CANCEL_CAPTION, hButton);
			case MB_OK:
				hButton = AddButton(hWnd, g_mbi->hInstance, IDOK, OK_CAPTION, hButton, true);
				g_nFirstButton = IDOK;
			break;
			case MB_RETRYCANCEL:
				hButton = AddButton(hWnd, g_mbi->hInstance, IDCANCEL, CANCEL_CAPTION, hButton);
				hButton = AddButton(hWnd, g_mbi->hInstance, IDRETRY, RETRY_CAPTION, hButton, true);
				g_nFirstButton = IDRETRY;
			break;
		}


	}
}

void OnPaint(HWND hWnd)
{
	PAINTSTRUCT ps;
	HDC hDC = BeginPaint(hWnd, &ps);
	HWND hControl;

	RECT rcDlg, rcControl;
	GetClientRect(hWnd, &rcDlg);
	HBRUSH hbrInfoBackground = CreateSolidBrush(RGB(255, 255, 255));
	hControl = GetDlgItem(hWnd, g_nFirstButton);

	GetClientRect(hControl, &rcControl);
	MapWindowPoints(hControl, hWnd, (LPPOINT)&rcControl, 2);
	rcDlg.bottom = rcControl.top-10;
	FillRect(hDC, &rcDlg, hbrInfoBackground);

	// ******************************************************
	// Draw icon

	ImageList_Draw(g_hImageList, g_mbi->uIconIndex, hDC, WMB_ICON_POSX, rcDlg.bottom / 2 - 32 / 2, ILD_NORMAL);

	// ******************************************************
	// Draw text

	HFONT hOldFont = (HFONT)SelectObject(hDC, g_hFont);

	rcDlg.left += PADDING_LEFT;
	DrawText(hDC, g_mbi->lpText, _tcslen(g_mbi->lpText), &rcDlg, DT_SINGLELINE | DT_VCENTER);

	SelectObject(hDC, hOldFont);

	// ******************************************************
	// Clean up

	DeleteObject(hbrInfoBackground);	
	EndPaint(hWnd, &ps);
}

INT_PTR CALLBACK DlgProc(HWND hWnd, UINT32 uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg) 
	{
	case WM_INITDIALOG:
		g_mbi = (LPMSGBOXINFO)lParam;
		OnCreateDlg(hWnd);
		return false;
	case WM_PAINT:
		OnPaint(hWnd);
		return true;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDOK:
		case IDYES:
		case IDNO:
		case IDCANCEL:
		case IDRETRY:
			if(GetDlgItem(hWnd, wParam) == NULL)
				return false;

			if(SendMessage(GetDlgItem(hWnd, IDC_CHECKBOX), BM_GETCHECK, 0, 0) > 0)
				wParam |= IDCHECKBOX;

			EndDialog(hWnd, wParam);
			return true;
		}
		return false;
	case WM_CLOSE:
		g_mbi = NULL;
		if(g_hOldFont != NULL) 
		{
			SelectObject(g_hDC, g_hOldFont);
			DeleteObject(g_hOldFont);
		}
		ReleaseDC(hWnd, g_hDC);
		EndDialog(hWnd, IDCANCEL);
		return true;
	}

	return false;
}

int WeeMessageBox(HWND hWnd, LPCTSTR lpszDlgRes, LPMSGBOXINFO mbi)
{
	return DialogBoxParam(mbi->hInstance, lpszDlgRes, hWnd, DlgProc, (LPARAM)mbi);
}
