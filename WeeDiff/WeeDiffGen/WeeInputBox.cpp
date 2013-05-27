#include "WeeInputBox.h"

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

WNDPROC g_OldEditProc = NULL;
HWND g_hInputWnd = NULL;

void OnInputReturn()
{
	SendMessage(g_hInputWnd, WM_COMMAND, ID_OK, 0);
}

LRESULT CALLBACK SubClassEditProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_KEYDOWN:
	case WM_GETDLGCODE:
		if(wParam == VK_RETURN)
		{
			OnInputReturn();
		}
		break;
	}

	return CallWindowProc(g_OldEditProc, hWnd, uMsg, wParam, lParam);
}

INT_PTR CALLBACK WeeInputBox::InputProc(HWND hWnd, UINT32 uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg) 
	{
	case WM_KEYDOWN:
	case WM_GETDLGCODE:
		if(wParam == VK_RETURN)
		{
			OnInputReturn();
		}
		break;
	case WM_INITDIALOG:
		{
			g_hInputWnd = hWnd;

			LPWEEINPUTINFO lpWII = (LPWEEINPUTINFO)lParam;

			SetWindowLong(hWnd, GWL_USERDATA, (LONG)lpWII);

			SIZE s;

			s.cx = 350;
			s.cy = 130;

			SetWindowPos(hWnd, NULL, (GetSystemMetrics(SM_CXSCREEN) - s.cx)/2, (GetSystemMetrics(SM_CYSCREEN) - s.cy)/2, s.cx, s.cy, SWP_NOZORDER);			

			HWND hEditBox = CreateWindowEx(WS_EX_CLIENTEDGE, WC_EDIT, 0, WS_CHILD | WS_VISIBLE | ES_WANTRETURN, 10, 10 + 20, s.cx - 25, 20, hWnd, (HMENU)ID_EDIT, NULL, NULL);
			SendMessage(hEditBox, WM_SETFONT, (WPARAM)lpWII->hFont, FALSE);

			HWND hColorButton = CreateWindowEx(0, WC_BUTTON, TEXT("Color"), WS_CHILD | WS_VISIBLE, 10, s.cy - 20 * 2 - 15, 75, 20, hWnd, (HMENU)ID_COLOR, NULL, NULL);
			SendMessage(hColorButton, WM_SETFONT, (WPARAM)lpWII->hFont, FALSE);

			HWND hOkButton = CreateWindowEx(0, WC_BUTTON, TEXT("OK"), WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, s.cx - 75 - 15, s.cy - 20 * 2 - 15, 75, 20, hWnd, (HMENU)ID_OK, NULL, NULL);
			SendMessage(hOkButton, WM_SETFONT, (WPARAM)lpWII->hFont, FALSE);

			g_OldEditProc = (WNDPROC)SetWindowLong(hEditBox, GWL_WNDPROC, (LONG)SubClassEditProc);
		}
		return true;
	case WM_PAINT:
		{
			LPWEEINPUTINFO lpWII = (LPWEEINPUTINFO)GetWindowLong(hWnd, GWL_USERDATA);

			PAINTSTRUCT ps;
			HDC hDC = BeginPaint(hWnd, &ps);

			RECT rc = {0};

			GetClientRect(hWnd, &rc);

			rc.left += 10;
			rc.right -= 10;
			rc.top += 10;
			rc.bottom -= 10;

			SetBkMode(hDC, TRANSPARENT);
			SelectObject(hDC, lpWII->hFont);

			DrawText(hDC, lpWII->lpszText, _tcslen(lpWII->lpszText), &rc, 0);

			EndPaint(hWnd, &ps);
		}
		return TRUE;
	case WM_COMMAND:
		{
			switch(wParam)
			{
			case ID_OK:
				{
					LPWEEINPUTINFO lpWII = (LPWEEINPUTINFO)GetWindowLong(hWnd, GWL_USERDATA);

					Edit_GetText(GetDlgItem(hWnd, ID_EDIT), lpWII->lpchDst, lpWII->uDstSize);
					EndDialog(hWnd, 0);
				}
				break;
			case ID_COLOR:
			{	
					LPWEEINPUTINFO lpWII = (LPWEEINPUTINFO)GetWindowLong(hWnd, GWL_USERDATA);
					CHOOSECOLOR sChooseColor;
					static COLORREF lpCustColors[16];
					static DWORD rgbCurrent = 0;

					Edit_GetText(GetDlgItem(hWnd, ID_EDIT), lpWII->lpchDst, lpWII->uDstSize);

					if(lpWII->lpchDst[0] == '#')
						rgbCurrent = _tcstol(lpWII->lpchDst + 1, NULL, 16);
					else
						rgbCurrent = _tcstol(lpWII->lpchDst, NULL, 16);

					rgbCurrent = ((rgbCurrent & 0x0000FF) << 16) + (rgbCurrent & 0x00FF00) + ((rgbCurrent & 0xFF0000) >> 16);

					ZeroMemory(&sChooseColor, sizeof(sChooseColor));
					sChooseColor.lStructSize = sizeof(sChooseColor);
					sChooseColor.hwndOwner = hWnd;
					sChooseColor.lpCustColors = (LPDWORD)lpCustColors;
					sChooseColor.rgbResult = rgbCurrent;
					sChooseColor.Flags = CC_FULLOPEN | CC_RGBINIT;

					if(ChooseColor(&sChooseColor) == TRUE) {
						rgbCurrent = sChooseColor.rgbResult;
					}

					_stprintf_s(lpWII->lpchDst, lpWII->uDstSize, TEXT("#%06X"), ((rgbCurrent & 0x0000FF) << 16) + (rgbCurrent & 0x00FF00) + ((rgbCurrent & 0xFF0000) >> 16));
					Edit_SetText(GetDlgItem(hWnd, ID_EDIT), lpWII->lpchDst);
				}
				break;
			}
		}
		break;
	case WM_CLOSE:
		EndDialog(hWnd, IDCANCEL);
		return true;
	}

	return false;
}

UINT32 WeeInputBox::DisplayInputBox(HWND hParent, LPCTSTR lpszCaption, LPCTSTR lpszText, LPTSTR lpchDst, UINT32 uDstSize, HFONT hFont)
{
	g_OldEditProc = NULL;
	g_hInputWnd = NULL;

	HGLOBAL hGlobal = CreateTemplate(lpszCaption);

	if(hGlobal == 0)
		return 0;

	WEEINPUTINFO sWII = {0};

	sWII.lpszText = (LPTSTR)lpszText;
	sWII.lpchDst = lpchDst;
	sWII.uDstSize = uDstSize;
	sWII.hFont = hFont;

	DialogBoxIndirectParam(NULL, (LPDLGTEMPLATE)hGlobal, hParent,  (DLGPROC)WeeInputBox::InputProc, (LPARAM)&sWII);
	GlobalFree(hGlobal);

	return 0;
}

HGLOBAL WeeInputBox::CreateTemplate(LPCTSTR lpszCaption)
{
	HGLOBAL hGlobal;
	LPDLGTEMPLATE lpDlgTmpl;
	LPWORD lpWord;
	LPWSTR lpszTitle;

	hGlobal = GlobalAlloc(GMEM_ZEROINIT, 1024);

	if(hGlobal == NULL)
		return 0;

	lpDlgTmpl = (LPDLGTEMPLATE)GlobalLock(hGlobal);

	lpDlgTmpl->style = WS_POPUP | WS_BORDER | DS_MODALFRAME | DS_FIXEDSYS | DS_CENTER | WS_CAPTION;

	lpWord = (LPWORD)(lpDlgTmpl + 1);
	*lpWord++ = 0;
	*lpWord++ = 0;

	lpszTitle = (LPWSTR)lpWord;

#ifndef UNICODE
	MultiByteToWideChar(CP_ACP, 0, lpszCaption, -1, lpszTitle, 50);
#else
	_tcsncpy_s(lpszTitle, 50, lpszCaption, 50);
#endif

	GlobalUnlock(hGlobal);
	return hGlobal;
}