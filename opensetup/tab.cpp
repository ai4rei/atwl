// -----------------------------------------------------------------
// RagnarokOnline OpenSetup
// (c) 2010 Ai4rei/AN
// See doc/license.txt for details.
// -----------------------------------------------------------------

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commctrl.h>

#include "resource.h"

#include "tab.h"

CTabMgr::~CTabMgr()
{
    this->Kill();
}

bool __stdcall CTabMgr::Init(HINSTANCE hInstance, HWND hWndParent)
{
    this->hInstance  = hInstance;
    this->hWndParent = hWndParent;
    this->hWnd       = CreateWindow(TOOLBARCLASSNAME, NULL, TBSTYLE_FLAT|TBSTYLE_LIST|WS_CHILD, 0, 0, 0, 0, hWndParent, NULL, hInstance, NULL);

    if(this->hWnd)
    {
        HBITMAP hBmp, hMsk;

        this->hImgList = ImageList_Create(16, 16, ILC_COLOR24|ILC_MASK, 0, IMI_MAX);
        hBmp = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_IMAGELIST), IMAGE_BITMAP, 0, 0, 0);
        hMsk = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_IMAGELIST_MASK), IMAGE_BITMAP, 0, 0, 0);
        ImageList_Add(this->hImgList, hBmp, hMsk);
        DeleteObject(hMsk);
        DeleteObject(hBmp);

        SendMessage(this->hWnd, TB_BUTTONSTRUCTSIZE, sizeof(TBBUTTON), 0);
        SendMessage(this->hWnd, TB_SETBITMAPSIZE, 0, MAKELONG(16, 16));
        SendMessage(this->hWnd, TB_SETIMAGELIST, 0, (LPARAM)this->hImgList);

        ShowWindow(this->hWnd, SW_SHOW);
        return true;
    }
    return false;
}
void __stdcall CTabMgr::Kill(void)
{
    this->hInstance  = NULL;
    this->hWndParent = NULL;
    if(this->hWnd)
    {
        DestroyWindow(this->hWnd);
        this->hWnd = NULL;
    }
    if(this->hImgList)
    {
        ImageList_Destroy(this->hImgList);
        this->hImgList = NULL;
    }
}

HWND __stdcall CTabMgr::AddTab(const char* lpszCaption, int nCmd, int nImgId, bool bEnabled, const char* lpszRes, DLGPROC lpfnProc)
{
    HWND hTab;

    if((hTab = CreateDialog(this->hInstance, lpszRes, this->hWndParent, lpfnProc))!=NULL)
    {
        TBBUTTON Button;

        Button.iBitmap = MAKELONG(nImgId,0);
        Button.fsStyle = BTNS_AUTOSIZE|BTNS_BUTTON|/*BTNS_CHECKGROUP|*/BTNS_SHOWTEXT;
        Button.fsState = TBSTATE_ENABLED;
        if(bEnabled)
        {
            //Button.fsState|= TBSTATE_CHECKED;
        }
        else
        {
            ShowWindow(hTab, SW_HIDE);
        }
        Button.idCommand = nCmd;
        Button.iString = (INT_PTR)lpszCaption;
        SendMessage(this->hWnd, TB_ADDBUTTONS, 1, (LPARAM)&Button);
        SendMessage(this->hWnd, TB_AUTOSIZE, 0, 0);
        return hTab;
    }
    return NULL;
}

void __stdcall CTabMgr::AddButton(const char* lpszCaption, int nCmd, int nImgId)
{
    TBBUTTON Button;

    Button.iBitmap = MAKELONG(nImgId,0);
    Button.fsStyle = BTNS_AUTOSIZE|BTNS_BUTTON|/*BTNS_CHECKGROUP|*/BTNS_SHOWTEXT;
    Button.fsState = TBSTATE_ENABLED;
    Button.idCommand = nCmd;
    Button.iString = (INT_PTR)lpszCaption;
    SendMessage(this->hWnd, TB_ADDBUTTONS, 1, (LPARAM)&Button);
    SendMessage(this->hWnd, TB_AUTOSIZE, 0, 0);
}
