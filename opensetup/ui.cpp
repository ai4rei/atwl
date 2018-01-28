// -----------------------------------------------------------------
// RagnarokOnline OpenSetup
// (c) 2010+ Ai4rei/AN
// See doc/license.txt for details.
//
// -----------------------------------------------------------------

#include <stdlib.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
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
        if(LoadStringA(GetModuleHandle(NULL), strtoul(szText+1, NULL, 10), szText, __ARRAYSIZE(szText)))
        {
            SetWindowTextA(hWnd, szText);
        }
    }

    return TRUE;
}

void __stdcall UI::LoadMUI(HWND hWnd)
{
    EnumChildWindows(hWnd, &UI::LoadMUIForEach, 0);
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
    Mbp.lpszIcon           = MAKEINTRESOURCE(IDI_MAINICON);
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

bool __stdcall UI::IsMirrorDriverPresent(bool* lpbActive)
{
    LPFNENUMDISPLAYDEVICESA EnumDisplayDevices = (LPFNENUMDISPLAYDEVICESA)GetProcAddress(GetModuleHandle("user32.dll"), "EnumDisplayDevicesA");

    if(EnumDisplayDevices)
    {
        unsigned long luIdx;
        DISPLAY_DEVICE DisplayDevice = { sizeof(DisplayDevice) };

        for(luIdx = 0; EnumDisplayDevices(NULL, luIdx, &DisplayDevice, 0); luIdx++)
        {
            if(DisplayDevice.StateFlags&DISPLAY_DEVICE_MIRRORING_DRIVER)
            {
                if(lpbActive)
                {
                    lpbActive[0] = (DisplayDevice.StateFlags&DISPLAY_DEVICE_ATTACHED_TO_DESKTOP)!=0;
                }

                return true;
            }
        }
    }

    if(lpbActive)
    {
        lpbActive[0] = false;
    }

    return false;
}

BOOL CALLBACK UI::EnableToolTipsForEach(HWND hWnd, LPARAM lParam)
{
    char szTestBuffer[16];
    HWND hWndTT = (HWND)lParam;
    HWND hWndMain = GetParent(hWndTT);
    HWND hWndParent = GetParent(hWnd);
    UINT uID = GetDlgCtrlID(hWnd);

    for(;;)
    {
        if(uID==0xffff)
        {// ignore labels and anonymous controls
            break;
        }

        if(!IsWindowEnabled(hWnd))
        {// do not display help for something disabled
            break;
        }

        if(hWndMain==hWndParent)
        {// does not have the tab window as immediate ancestor
            break;
        }

        if(UI::GetSizeRatio(hWndParent, hWnd)>=75)
        {// too large, to be intended target
            break;
        }

        if(!LoadStringA(GetWindowInstance(hWndParent), uID, szTestBuffer, __ARRAYSIZE(szTestBuffer)))
        {// no such string
            break;
        }

        {// add tool
            TOOLINFO Ti = { sizeof(Ti) };

            Ti.uFlags = TTF_IDISHWND|TTF_SUBCLASS;
            Ti.hwnd = hWndParent;
            Ti.uId = (UINT_PTR)hWnd;
            Ti.lpszText = LPSTR_TEXTCALLBACK;

            SendMessage(hWndTT, TTM_ADDTOOL, 0, (LPARAM)&Ti);
        }
        break;
    }

    return TRUE;
}

void __stdcall UI::EnableToolTips(HWND hWnd, BOOL bEnable)
{
    static HWND hWndTT = NULL;

    if(!hWndTT)
    {
        RECT rcWnd;

        // first time initialization
        hWndTT = CreateWindowEx(0, TOOLTIPS_CLASS, NULL, TTS_NOPREFIX|TTS_ALWAYSTIP, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, hWnd, NULL, GetWindowInstance(hWnd), NULL);

        // limit the tool tip width to the width of the main window
        GetWindowRect(hWnd, &rcWnd);
        SendMessage(hWndTT, TTM_SETMAXTIPWIDTH, 0, rcWnd.right-rcWnd.left);
    }

    if(bEnable)
    {
        // add all matching child windows
        EnumChildWindows(hWnd, &UI::EnableToolTipsForEach, (LPARAM)hWndTT);
    }
    else
    {
        // clear all tools
        TOOLINFO Ti = { sizeof(Ti) };

        while(SendMessage(hWndTT, TTM_ENUMTOOLS, 0, (LPARAM)&Ti))
        {
            SendMessage(hWndTT, TTM_DELTOOL, 0, (LPARAM)&Ti);
        }
    }

    // finally change state
    SendMessage(hWndTT, TTM_ACTIVATE, bEnable, 0);
}

void __stdcall UI::HandleToolTips(LPNMHDR lpHdr)
{
    static char szLastMsg[4096+1] = { 0 };
    UINT uID;

    if(lpHdr->code==TTN_GETDISPINFO)
    {
        LPNMTTDISPINFO lpDi = (LPNMTTDISPINFO)lpHdr;

        if(lpDi->szText[0])
        {// auto tick-value from trackbar
            return;
        }

        if(lpDi->uFlags&TTF_IDISHWND)
        {
            uID = GetDlgCtrlID((HWND)lpHdr->idFrom);
        }
        else
        {
            uID = lpHdr->idFrom;
        }

        if(!LoadStringA(GetWindowInstance(GetParent(lpHdr->hwndFrom)), uID, szLastMsg, __ARRAYSIZE(szLastMsg)))
        {// no such string, should not happen
            DebugBreakHere();
            return;
        }

        lpDi->lpszText = szLastMsg;
        lpDi->hinst = NULL;
    }
}

HFONT __stdcall UI::GetShellFont()
{
    NONCLIENTMETRICS Ncm = { sizeof(Ncm) };

    if(SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(Ncm), &Ncm, 0))
    {
        HFONT hFont = CreateFontIndirect(&Ncm.lfMessageFont);

        if(hFont)
        {
            return hFont;
        }
    }

    return (HFONT)GetStockObject(SYSTEM_FONT);
}
