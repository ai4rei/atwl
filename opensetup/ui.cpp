// -----------------------------------------------------------------
// RagnarokOnline OpenSetup
// (c) 2010-2013 Ai4rei/AN
// See doc/license.txt for details.
//
// -----------------------------------------------------------------

#include <stdlib.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commctrl.h>
#include <htmlhelp.h>

#include "opensetup.h"
#include "resource.h"
#include "ui.h"

void __stdcall UI::BatchMessage(HWND hWnd, LPCBATCHLIST lpBatchList, unsigned long luItems)
{
    unsigned long i;

    for(i = 0; i<luItems; i++)
    {
        SendMessage(GetDlgItem(hWnd, lpBatchList[i].nIDDlgItem), lpBatchList[i].uMsg, lpBatchList[i].wParam, lpBatchList[i].lParam);
    }
}

void __stdcall UI::FillComboBox(HWND hWnd, int nId, const char** lppszList, unsigned long luItems)
{
    unsigned long i;
    HWND hWndItem = GetDlgItem(hWnd, nId);

    // reset previously assigned strings
    SendMessage(hWndItem, CB_RESETCONTENT, 0, 0);

    for(i = 0; i<luItems; i++)
    {
        SendMessage(hWndItem, CB_ADDSTRING, 0, (LPARAM)lppszList[i]);
    }
}

void __stdcall UI::FillComboBoxMUI(HWND hWnd, int nId, const unsigned int* lpuList, unsigned long luItems)
{
    char szBuffer[1024];
    unsigned long i;
    HWND hWndItem = GetDlgItem(hWnd, nId);

    // reset previously assigned strings
    SendMessage(hWndItem, CB_RESETCONTENT, 0, 0);

    for(i = 0; i<luItems; i++)
    {
        LoadStringA(GetModuleHandle(NULL), lpuList[i], szBuffer, __ARRAYSIZE(szBuffer));
        SendMessage(hWndItem, CB_ADDSTRING, 0, (LPARAM)szBuffer);
    }
}

BOOL CALLBACK UI::LoadMUIForEach(HWND hWnd, LPARAM lParam)
{
    char szText[2048];

    if(GetWindowTextA(hWnd, szText, __ARRAYSIZE(szText)) && szText[0]=='#')
    {
        if(LoadStringA((HINSTANCE)lParam, strtoul(szText+1, NULL, 10), szText, __ARRAYSIZE(szText)))
        {
            SetWindowTextA(hWnd, szText);
        }
    }

    return TRUE;
}

void __stdcall UI::LoadMUI(HWND hWnd)
{
    EnumChildWindows(hWnd, &UI::LoadMUIForEach, (LPARAM)GetModuleHandle(NULL));
}

int __stdcall UI::MessageBoxEx(HWND hWnd, const char* lpszText, const char* lpszCaption, unsigned long luStyle)
{
    MSGBOXPARAMS Mbp;

    Mbp.cbSize             = sizeof(Mbp);
    Mbp.hwndOwner          = hWnd;
    Mbp.hInstance          = GetModuleHandle(NULL);
    Mbp.lpszText           = lpszText;
    Mbp.lpszCaption        = lpszCaption;
    Mbp.dwStyle            = luStyle|MB_USERICON;
    Mbp.lpszIcon           = MAKEINTRESOURCE(IDI_APPLICATION_LARGE);
    Mbp.dwContextHelpId    = 0;
    Mbp.lpfnMsgBoxCallback = NULL;
    Mbp.dwLanguageId       = 0;

    return MessageBoxIndirect(&Mbp);
}

unsigned long __stdcall UI::GetSizeRatio(HWND hWndA, HWND hWndB)
{// A: 100%, B: Part
    unsigned long luSA, luSB;
    RECT rcA, rcB;

    GetWindowRect(hWndA, &rcA);
    GetWindowRect(hWndB, &rcB);

    luSA = (rcA.right-rcA.left)*(rcA.bottom-rcA.top);
    luSB = (rcB.right-rcB.left)*(rcB.bottom-rcB.top);

    return (100*luSB)/luSA;
}

void __stdcall UI::SetButtonShield(HWND hWnd, bool bEnable)
{
#ifndef BCM_SETSHIELD
    #ifndef BCM_FIRST
        #define BCM_FIRST 0x1600
    #endif  /* BCM_FIRST */
    #define BCM_SETSHIELD (BCM_FIRST+0x000C)
#endif  /* BCM_SETSHIELD */
    SendMessage(hWnd, BCM_SETSHIELD, 0, (LPARAM)bEnable);
}

BOOL CALLBACK UI::SetFontForEach(HWND hWnd, LPARAM lParam)
{
    SendMessage(hWnd, WM_SETFONT, (WPARAM)lParam, MAKELPARAM(FALSE, 0));

    return TRUE;
}

void __stdcall UI::SetFont(HWND hWnd, HFONT hFont)
{
    EnumChildWindows(hWnd, &UI::SetFontForEach, (LPARAM)hFont);
    InvalidateRect(hWnd, NULL, TRUE);  // reflect change
}

bool __stdcall UI::GetCheckBoxTick(HWND hWnd, int nId)
{
    return IsDlgButtonChecked(hWnd, nId)!=BST_UNCHECKED;
}

void __stdcall UI::HHLite(HWND hWnd, LPHELPINFO lpHi)
{
    static char szLastMsg[4096+1] = { 0 };
    HH_POPUP Popup;

    if(lpHi->cbSize<sizeof(HELPINFO))
    {// less data than we expect
        return;
    }

    if(lpHi->iContextType!=HELPINFO_WINDOW)
    {// do not care about menu help (there should not be any, either)
        return;
    }

    if(lpHi->iCtrlId==0xffff)
    {// ignore labels
        return;
    }

    if(!IsWindowEnabled((HWND)lpHi->hItemHandle))
    {// do not display help for something disabled
        return;
    }

    if(hWnd!=GetParent((HWND)lpHi->hItemHandle))
    {// does not have the current parent as immediate ancestor
        return;
    }

    if(UI::GetSizeRatio(hWnd, (HWND)lpHi->hItemHandle)>=75)
    {// too large, to be intended target
        return;
    }

    if(!LoadStringA(GetModuleHandle(NULL), lpHi->iCtrlId, szLastMsg, __ARRAYSIZE(szLastMsg)))
    {// no such string
        return;
    }

    Popup.cbStruct = sizeof(Popup);
    Popup.hinst = NULL;  // GetModuleHandle(NULL);
    Popup.idString = 0;  // lpHi->iCtrlId;
    Popup.pszText = szLastMsg;
    Popup.pt.x = lpHi->MousePos.x;     // has no effect
    Popup.pt.y = lpHi->MousePos.y+16;  // NOTE: offset coords so that the window does not cover the control completely
    Popup.clrForeground = -1;
    Popup.clrBackground = -1;
    Popup.rcMargins.left =
    Popup.rcMargins.top =
    Popup.rcMargins.right =
    Popup.rcMargins.bottom = -1;
    Popup.pszFont = "Tahoma, 8";
    HtmlHelp((HWND)lpHi->hItemHandle, NULL, HH_DISPLAY_TEXT_POPUP, (DWORD)&Popup);
}

BOOL CALLBACK UI::SetFocusFirstChildEach(HWND hWnd, LPARAM lParam)
{
    if(IsWindowEnabled(hWnd) /* enabled */ && IsWindowVisible(hWnd) /* visible */ && (GetWindowLong(hWnd, GWL_STYLE)&WS_TABSTOP) /* is tabbable */)
    {
        SetFocus(hWnd);
        return FALSE;
    }

    return TRUE;
}

void __stdcall UI::SetFocusFirstChild(HWND hWnd)
{
    EnumChildWindows(hWnd, &UI::SetFocusFirstChildEach, 0);
}

bool __stdcall UI::IsRemoteSession(void)
{
    return GetSystemMetrics(SM_REMOTESESSION)!=0;
}
