// -----------------------------------------------------------------
// RagnarokOnline OpenSetup
// (c) 2010 Ai4rei/AN
// See doc/license.txt for details.
// -----------------------------------------------------------------

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "resource.h"
#include "ui.h"

void __stdcall UI::BatchMessage(HWND hWnd, struct UIBatchList* lpBatchList, unsigned long luItems)
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

    for(i = 0; i<luItems; i++)
    {
        SendMessage(hWndItem, CB_ADDSTRING, 0, (LPARAM)lppszList[i]);
    }
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
