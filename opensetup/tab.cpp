// -----------------------------------------------------------------
// RagnarokOnline OpenSetup
// (c) 2010-2014 Ai4rei/AN
// See doc/license.txt for details.
//
// -----------------------------------------------------------------

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commctrl.h>

#include "tab.h"
#include "ui.h"

#define IMGLISTID_MASK(x) MAKEINTRESOURCE((x)+0)
#define IMGLISTID_24BP(x) MAKEINTRESOURCE((x)+2)
#define IMGLISTID_32BP(x) MAKEINTRESOURCE((x)+3)

bool __stdcall CTabMgr::IsTransparencyBitmapSupportBroken(void)
{
    OSVERSIONINFO Osvi = { sizeof(Osvi) };

    ::GetVersionEx(&Osvi);

    return (Osvi.dwPlatformId==VER_PLATFORM_WIN32_NT) && (Osvi.dwMajorVersion<5 || (Osvi.dwMajorVersion==5 && Osvi.dwMinorVersion<1));  // Windows 2000 and earlier
}

void __stdcall CTabMgr::InitImgList(int nImgDimension, int nImgListBase, int nMaxImages)
{
    HBITMAP hBmp, hMsk = NULL;
    UINT uFlags = ILC_COLOR32;

    if(IsTransparencyBitmapSupportBroken() || (hBmp = (HBITMAP)::LoadImage(m_hInstance, IMGLISTID_32BP(nImgListBase), IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION))==NULL)
    {// GetLastError will return 120 (ERROR_CALL_NOT_SUPPORTED)
        hBmp = (HBITMAP)::LoadImage(m_hInstance, IMGLISTID_24BP(nImgListBase), IMAGE_BITMAP, 0, 0, 0);
        hMsk = (HBITMAP)::LoadImage(m_hInstance, IMGLISTID_MASK(nImgListBase), IMAGE_BITMAP, 0, 0, 0);
        uFlags = ILC_COLOR24|ILC_MASK;
    }

    m_hImgList = ::ImageList_Create(nImgDimension, nImgDimension, uFlags, 0, nMaxImages);

    ::ImageList_Add(m_hImgList, hBmp, hMsk);

    ::DeleteObject(hMsk);
    ::DeleteObject(hBmp);
}

bool __stdcall CTabMgr::GetActiveTab(unsigned long& luIdx)
{
    for(luIdx = 0; luIdx<m_TabList.size() && !::SendMessage(m_hWnd, TB_ISBUTTONCHECKED, m_TabList[luIdx].first, 0); luIdx++);

    if(luIdx==m_TabList.size())
    {
        return false;
    }

    return true;
}

CTabMgr::CTabMgr()
{
    m_hImgList   = NULL;
    m_hInstance  = NULL;
    m_hWnd       = NULL;
    m_hWndParent = NULL;
}

CTabMgr::~CTabMgr()
{
    Kill();
}

bool __stdcall CTabMgr::Init(HINSTANCE hInstance, HWND hWndParent, int nImgDimension, int nImgListBase, int nMaxImages)
{
    m_hInstance  = hInstance;
    m_hWndParent = hWndParent;
    m_hWnd       = ::CreateWindow(TOOLBARCLASSNAME, NULL, TBSTYLE_FLAT|TBSTYLE_LIST|WS_CHILD, 0, 0, 0, 0, hWndParent, NULL, hInstance, NULL);

    if(m_hWnd)
    {
        InitImgList(nImgDimension, nImgListBase, nMaxImages);

        ::SendMessage(m_hWnd, TB_BUTTONSTRUCTSIZE, sizeof(TBBUTTON), 0);
        ::SendMessage(m_hWnd, TB_SETBITMAPSIZE, 0, MAKELONG(nImgDimension, nImgDimension));
        ::SendMessage(m_hWnd, TB_SETIMAGELIST, 0, (LPARAM)m_hImgList);

        ::ShowWindow(m_hWnd, SW_SHOW);

        return true;
    }

    return false;
}

void __stdcall CTabMgr::Kill(void)
{
    m_hInstance  = NULL;
    m_hWndParent = NULL;

    if(m_hWnd)
    {
        ::DestroyWindow(m_hWnd);
        m_hWnd = NULL;
    }

    if(m_hImgList)
    {
        ::ImageList_Destroy(m_hImgList);
        m_hImgList = NULL;
    }
}

HWND __stdcall CTabMgr::AddTab(const char* lpszCaption, int nCmd, int nImgId, bool bEnabled, const char* lpszRes, DLGPROC lpfnProc)
{
    HWND hTab;

    if((hTab = ::CreateDialog(m_hInstance, lpszRes, m_hWndParent, lpfnProc))!=NULL)
    {
        TBBUTTON Button;

        Button.iBitmap = MAKELONG(nImgId,0);
        Button.fsStyle = BTNS_AUTOSIZE|BTNS_BUTTON|BTNS_CHECKGROUP|BTNS_SHOWTEXT;
        Button.fsState = TBSTATE_ENABLED;

        if(bEnabled)
        {
            Button.fsState|= TBSTATE_CHECKED;
            UI::SetFocusFirstChild(hTab);
        }
        else
        {
            ::ShowWindow(hTab, SW_HIDE);
        }

        Button.idCommand = nCmd;
        Button.iString = (INT_PTR)lpszCaption;

        ::SendMessage(m_hWnd, TB_ADDBUTTONS, 1, (LPARAM)&Button);

        m_TabList.push_back(TABITEM(nCmd, hTab));

        OnResizeMove();

        return hTab;
    }

    return NULL;
}

HWND __stdcall CTabMgr::GetTab(int nCmd)
{
    unsigned long luIdx;

    for(luIdx = 0; luIdx<m_TabList.size(); luIdx++)
    {
        if(m_TabList[luIdx].first==nCmd)
        {
            return m_TabList[luIdx].second;
        }
    }

    return NULL;
}

void __stdcall CTabMgr::AddButton(const char* lpszCaption, int nCmd, int nImgId)
{
    TBBUTTON Button;

    Button.iBitmap = MAKELONG(nImgId,0);
    Button.fsStyle = BTNS_AUTOSIZE|BTNS_BUTTON|BTNS_CHECKGROUP|BTNS_SHOWTEXT;
    Button.fsState = TBSTATE_ENABLED;
    Button.idCommand = nCmd;
    Button.iString = (INT_PTR)lpszCaption;

    ::SendMessage(m_hWnd, TB_ADDBUTTONS, 1, (LPARAM)&Button);

    OnResizeMove();
}

void __stdcall CTabMgr::OnResizeMove(void)
{
    ::SendMessage(m_hWnd, TB_AUTOSIZE, 0, 0);
}

void __stdcall CTabMgr::OnActivateTab(int nCmd, bool bManual)
{
    unsigned long luIdx;

    for(luIdx = 0; luIdx<m_TabList.size(); luIdx++)
    {
        if(m_TabList[luIdx].first==nCmd)
        {
            if(bManual)
            {
                ::SendMessage(m_hWnd, TB_CHECKBUTTON, m_TabList[luIdx].first, MAKELONG(TRUE, 0));
            }

            ::ShowWindow(m_TabList[luIdx].second,  SW_SHOW);

            UI::SetFocusFirstChild(m_TabList[luIdx].second);
        }
        else
        {
            if(bManual)
            {
                ::SendMessage(m_hWnd, TB_CHECKBUTTON, m_TabList[luIdx].first, MAKELONG(FALSE, 0));
            }

            ::ShowWindow(m_TabList[luIdx].second,  SW_HIDE);
        }
    }
}

void __stdcall CTabMgr::ActivateTabNext(void)
{
    unsigned long luIdx = 0;

    if(!GetActiveTab(luIdx))
    {
        return;
    }
    else if(luIdx==m_TabList.size()-1)
    {
        OnActivateTab(m_TabList[0].first, true);
    }
    else
    {
        OnActivateTab(m_TabList[luIdx+1].first, true);
    }
}

void __stdcall CTabMgr::ActivateTabPrev(void)
{
    unsigned long luIdx = 0;

    if(!GetActiveTab(luIdx))
    {
        return;
    }
    else if(!luIdx)
    {
        OnActivateTab(m_TabList[m_TabList.size()-1].first, true);
    }
    else
    {
        OnActivateTab(m_TabList[luIdx-1].first, true);
    }
}

HWND __stdcall CTabMgr::GetWindowHandle(void)
{
    return m_hWnd;
}
