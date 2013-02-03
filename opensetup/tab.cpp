// -----------------------------------------------------------------
// RagnarokOnline OpenSetup
// (c) 2010-2013 Ai4rei/AN
// See doc/license.txt for details.
//
// -----------------------------------------------------------------

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commctrl.h>

#include "tab.h"

CTabMgr::~CTabMgr()
{
    this->Kill();
}

bool __stdcall CTabMgr::Init(HINSTANCE hInstance, HWND hWndParent, int nImgDimension, const char* lpszImgList32Id, const char* lpszImgListId, const char* lpszImgListMaskId, int nMaxImages)
{
    this->m_hInstance  = hInstance;
    this->m_hWndParent = hWndParent;
    this->m_hWnd       = CreateWindow(TOOLBARCLASSNAME, NULL, TBSTYLE_FLAT|TBSTYLE_LIST|WS_CHILD, 0, 0, 0, 0, hWndParent, NULL, hInstance, NULL);

    if(this->m_hWnd)
    {
        bool bSupportAlphaBitmap = true;
        HBITMAP hBmp, hMsk = NULL;

        if(HasBrokenTransparencyBitmapSupport() || (hBmp = (HBITMAP)LoadImage(hInstance, lpszImgList32Id, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION))==NULL)
        {// GetLastError will return 120 (ERROR_CALL_NOT_SUPPORTED)
            hBmp = (HBITMAP)LoadImage(hInstance, lpszImgListId, IMAGE_BITMAP, 0, 0, 0);
            hMsk = (HBITMAP)LoadImage(hInstance, lpszImgListMaskId, IMAGE_BITMAP, 0, 0, 0);
            bSupportAlphaBitmap = false;
        }

        this->m_hImgList = ImageList_Create(nImgDimension, nImgDimension, bSupportAlphaBitmap ? ILC_COLOR32 : ILC_COLOR24|ILC_MASK, 0, nMaxImages);
        ImageList_Add(this->m_hImgList, hBmp, hMsk);
        DeleteObject(hMsk);
        DeleteObject(hBmp);

        SendMessage(this->m_hWnd, TB_BUTTONSTRUCTSIZE, sizeof(TBBUTTON), 0);
        SendMessage(this->m_hWnd, TB_SETBITMAPSIZE, 0, MAKELONG(nImgDimension, nImgDimension));
        SendMessage(this->m_hWnd, TB_SETIMAGELIST, 0, (LPARAM)this->m_hImgList);

        ShowWindow(this->m_hWnd, SW_SHOW);

        return true;
    }

    return false;
}

void __stdcall CTabMgr::Kill(void)
{
    this->m_hInstance  = NULL;
    this->m_hWndParent = NULL;
    if(this->m_hWnd)
    {
        DestroyWindow(this->m_hWnd);
        this->m_hWnd = NULL;
    }
    if(this->m_hImgList)
    {
        ImageList_Destroy(this->m_hImgList);
        this->m_hImgList = NULL;
    }
}

HWND __stdcall CTabMgr::AddTab(const char* lpszCaption, int nCmd, int nImgId, bool bEnabled, const char* lpszRes, DLGPROC lpfnProc)
{
    HWND hTab;

    if((hTab = CreateDialog(this->m_hInstance, lpszRes, this->m_hWndParent, lpfnProc))!=NULL)
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
        SendMessage(this->m_hWnd, TB_ADDBUTTONS, 1, (LPARAM)&Button);
        this->m_TabList.push_back(std::pair< int, HWND >(nCmd, hTab));
        this->OnResizeMove();

        return hTab;
    }

    return NULL;
}

HWND __stdcall CTabMgr::GetTab(int nCmd)
{
    unsigned long luIdx;

    for(luIdx = 0; luIdx<this->m_TabList.size(); luIdx++)
    {
        if(this->m_TabList[luIdx].first==nCmd)
        {
            return this->m_TabList[luIdx].second;
        }
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
    SendMessage(this->m_hWnd, TB_ADDBUTTONS, 1, (LPARAM)&Button);
    this->OnResizeMove();
}

void __stdcall CTabMgr::OnResizeMove(void)
{
    SendMessage(this->m_hWnd, TB_AUTOSIZE, 0, 0);
}

void __stdcall CTabMgr::OnActivateTab(int nCmd)
{
    unsigned long luIdx;

    for(luIdx = 0; luIdx<this->m_TabList.size(); luIdx++)
    {
        ShowWindow(this->m_TabList[luIdx].second, (this->m_TabList[luIdx].first==nCmd) ? SW_SHOW : SW_HIDE);
    }
}

HWND __stdcall CTabMgr::GetWindowHandle(void)
{
    return this->m_hWnd;
}

bool __stdcall CTabMgr::HasBrokenTransparencyBitmapSupport(void)
{
    OSVERSIONINFO Osvi = { sizeof(Osvi) };

    GetVersionEx(&Osvi);

    return (Osvi.dwPlatformId==VER_PLATFORM_WIN32_NT) && (Osvi.dwMajorVersion<5 || (Osvi.dwMajorVersion==5 && Osvi.dwMinorVersion<1));
}
