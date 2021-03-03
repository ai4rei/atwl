// -----------------------------------------------------------------
// RO Credentials (ROCred)
// (c) 2012+ Ai4rei/AN
//
// -----------------------------------------------------------------

#include <stdio.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>

#include <btypes.h>
#include <bvcstr.h>
#include <bvllst.h>
#include <bvpars.h>
#include <memtaf.h>
#include <regionui.h>
#include <w32ex.h>
#include <w32ui.h>
#include <w32uxt.h>

#include "button.h"
#include "config.h"
#include "rocred.h"

#include "bgskin.h"

BEGINSTRUCT(BUTTONSKININFO)
{
    BVLLISTNODE Node;
    UINT uID;
    HBITMAP hbmLook;
}
CLOSESTRUCT(BUTTONSKININFO);

static HBITMAP l_hbmBackground = NULL;
static HBRUSH l_hbrEditBack = NULL;
static BVLLIST l_SkinList = { 0 };  // id -> LPBGSKININFO

static const char* __WDECL BgSkin_P_ButtonId2Name(UINT uId)
{
    switch(uId)
    {
    C2N(IDOK);
    C2N(IDCANCEL);
    C2N(IDS_USERNAME);
    C2N(IDC_USERNAME);
    C2N(IDS_PASSWORD);
    C2N(IDC_PASSWORD);
    C2N(IDC_CHECKSAVE);
    }

    return NULL;
}

static unsigned char __WDECL BgSkin_P_ButtonState2Index(UINT uState)
{
    unsigned char ucIndex = 0;

    if(uState&ODS_SELECTED)
    {
        ucIndex = 2;
    }
    else if(uState&ODS_FOCUS)
    {
        ucIndex = 1;
    }

    return ucIndex;
}

static HBITMAP __WDECL BgSkin_P_GetSkin(UINT uID)
{
    LPBVLLISTNODE Item;

    for(Item = l_SkinList.Head; Item!=NULL; Item = Item->Next)
    {
        LPCBUTTONSKININFO const lpBsi = BvLListNodeParent(Item, BUTTONSKININFO, Node);

        if(lpBsi->uID==uID)
        {
            return lpBsi->hbmLook;
        }
    }

    return NULL;
}

static void __WDECL BgSkin_P_MakeLocalFileName(char const* const lpszName, char* const lpszBuffer, size_t const uBufferSize)
{
    char szBaseName[MAX_PATH];

    BvStrNCpyA(szBaseName, lpszName, __ARRAYSIZE(szBaseName));
    BvStrToLowerA(szBaseName);
    GetModuleFileNameSpecificPathA(NULL, lpszBuffer, uBufferSize, szBaseName, "bmp");
}

static HBITMAP __WDECL BgSkin_P_LoadBitmap(const char* lpszFileName, const char* lpszImageName)
{
    HBITMAP hBitmap;

    // try embedded
    if((hBitmap = LoadImageA(GetModuleHandle(NULL), lpszImageName, IMAGE_BITMAP, 0, 0, 0))==NULL)
    {
        // fall back to local file, but do not actually care about the result
        hBitmap = LoadImageA(NULL, lpszFileName, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    }

    return hBitmap;
}

static HBITMAP __WDECL BgSkin_P_LoadBitmap2(char const* const lpszImageName)
{
    char szLocalFile[MAX_PATH];

    BgSkin_P_MakeLocalFileName(lpszImageName, szLocalFile, __ARRAYSIZE(szLocalFile));

    return BgSkin_P_LoadBitmap(szLocalFile, lpszImageName);
}

static bool __WDECL BgSkin_P_IsActive(void)
{
    return l_hbmBackground!=NULL;
}

bool __WDECL BgSkinOnEraseBkGnd(HWND hWnd, HDC hDC)
{
    RECT rcWnd;
    HDC hdcBitmap;

    if(BgSkin_P_IsActive())
    {
        if(GetClientRect(hWnd, &rcWnd))
        {
            if((hdcBitmap = CreateCompatibleDC(hDC))!=NULL)
            {
                HGDIOBJ hGdiObj = SelectObject(hdcBitmap, l_hbmBackground);

                BitBlt(hDC, 0, 0, rcWnd.right-rcWnd.left, rcWnd.bottom-rcWnd.top, hdcBitmap, 0, 0, SRCCOPY);

                SelectObject(hdcBitmap, hGdiObj);
                DeleteDC(hdcBitmap);
                return true;
            }
        }
    }

    return false;
}

bool __WDECL BgSkinOnLButtonDown(HWND hWnd)
{
    if(BgSkin_P_IsActive() && !ConfigGetInt("ShowWindowCaption"))
    {
        // enable moving everywhere, since we no longer have a caption
        SendMessage(hWnd, WM_NCLBUTTONDOWN, HTCAPTION, 0);
        return true;
    }

    return false;
}

HBRUSH __WDECL BgSkinOnCtlColorStatic(HDC hDC, HWND hWnd)
{
    if(BgSkin_P_IsActive())
    {
        SetBkMode(hDC, TRANSPARENT);

        return GetStockObject(NULL_BRUSH);
    }

    return NULL;
}

HBRUSH __WDECL BgSkinOnCtlColorEdit(HDC hDC, HWND hWnd)
{
    if(BgSkin_P_IsActive())
    {
        switch(ConfigGetInt("EditBackground"))
        {
        case 1:
            SetBkMode(hDC, TRANSPARENT);

            // BUG: Transparency does not work on Windows XP+
            //
            // COMCTL32 V6 introduced in Windows XP employs
            // unconditional double buffering (in this case
            // CCBeginDoubleBuffer/CCEndDoubleBuffer in
            // EditSL_Paint), but since transparency with
            // WS_EX_TRANSPARENT and the NULL_BRUSH rely on
            // nothing being drawn, the double-buffering bitmap
            // remains blank (black), which is then pasted over
            // the to-be transparent background.
            // Currently the only workaround is to use the old
            // common controls built into USER32 (basically
            // abandon visual styles altogether).
            return GetStockObject(NULL_BRUSH);
        case 2:
            SetBkMode(hDC, TRANSPARENT);

            SetTextColor(hDC, BvStrToRgbA(ConfigGetStr("EditForegroundColor"), NULL));

            if(l_hbrEditBack!=NULL)
            {
                DeleteBrush(l_hbrEditBack);
            }

            l_hbrEditBack = CreateSolidBrush(BvStrToRgbA(ConfigGetStr("EditBackgroundColor"), NULL));

            return l_hbrEditBack;
        }
    }

    return NULL;
}

bool __WDECL BgSkinOnDrawItem(UINT uID, const DRAWITEMSTRUCT* lpDis)
{
    HBITMAP hbmLook;

    if((hbmLook = BgSkin_P_GetSkin(uID))!=NULL)
    {
        HDC hdcBitmap;

        if((hdcBitmap = CreateCompatibleDC(lpDis->hDC))!=NULL)
        {
            int const nWidth = lpDis->rcItem.right-lpDis->rcItem.left;
            int const nHeight = lpDis->rcItem.bottom-lpDis->rcItem.top;
            HGDIOBJ hGdiObj = SelectObject(hdcBitmap, hbmLook);

            BitBlt(lpDis->hDC, lpDis->rcItem.left, lpDis->rcItem.top, nWidth, nHeight, hdcBitmap, lpDis->rcItem.left+BgSkin_P_ButtonState2Index(lpDis->itemState)*nWidth, lpDis->rcItem.top, SRCCOPY);

            SelectObject(hdcBitmap, hGdiObj);
            DeleteDC(hdcBitmap);
            return true;
        }
    }

    return false;
}

static void __WDECL BgSkin_P_RegisterButtonSkin(unsigned int uBtnId, const char* lpszName)
{
    HBITMAP hbmLook;

    hbmLook = BgSkin_P_LoadBitmap2(lpszName);

    if(hbmLook!=NULL)
    {
        LPBUTTONSKININFO lpBsi = NULL;

        if(MemTAlloc(&lpBsi))
        {
            BvLListNodeInit(&lpBsi->Node);

            lpBsi->uID     = uBtnId;
            lpBsi->hbmLook = hbmLook;

            BvLListInsert(&l_SkinList, &lpBsi->Node);
        }
        else
        {
            DeleteBitmap(hbmLook);
        }
    }
}

bool __WDECL BgSkinInit(HWND hWnd)
{
    BITMAP bmBG;
    HWND hChildWnd = NULL;

    l_hbmBackground = BgSkin_P_LoadBitmap2("BGSKIN");

    if(l_hbmBackground!=NULL)
    {
        if(GetObject(l_hbmBackground, sizeof(bmBG), &bmBG))
        {
            RECT rcWnd;

            // set window size to bitmap size
            SetRect(&rcWnd, 0, 0, bmBG.bmWidth, bmBG.bmHeight);

            if(ConfigGetInt("ShowWindowCaption"))
            {// adjust rectangle to account for non-client elements
                AdjustWindowRectEx(&rcWnd, GetWindowStyle(hWnd), GetMenu(hWnd)!=NULL, GetWindowExStyle(hWnd));
            }
            else
            {// get rid of non-client elements
                SetWindowLongPtr(hWnd, GWL_STYLE, WS_POPUP|WS_SYSMENU|WS_MINIMIZEBOX);
                SetWindowPos(hWnd, NULL, 0, 0, 0, 0, SWP_FRAMECHANGED|SWP_NOACTIVATE|SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER|SWP_NOOWNERZORDER);
            }

            // center window and apply size
            CenterWindowInWorkAreaWithRect(hWnd, &rcWnd);

            // only chrome-less windows can use irregular shapes
            if(!ConfigGetInt("ShowWindowCaption"))
            {
                HRGN hRGN;

                // window size for base window region (origin transform)
                GetClientRect(hWnd, &rcWnd);

                // set up region
                if((hRGN = CreateRectRgnIndirect(&rcWnd))!=NULL)
                {
                    if(MaskRegionFromBitmap(hRGN, l_hbmBackground, RGB(255, 0, 255)))
                    {
                        SetWindowRgn(hWnd, hRGN, TRUE);
                        hRGN = NULL;  // system took ownership
                    }
                    else
                    {
                        DeleteRgn(hRGN);
                    }
                }
            }
        }

        // process all child windows
        while((hChildWnd = FindWindowExA(hWnd, hChildWnd, NULL, NULL))!=NULL)
        {
            char szBuffer[128];
            const char* lpszName;
            unsigned int uBtnId = GetDlgCtrlID(hChildWnd);
            int nX, nY, nW, nH;
            HBITMAP hbmLook;

            lpszName = BgSkin_P_ButtonId2Name(uBtnId);

            // disable visual styles to prevent glitches
            W32UxTheme_DisableVisualStyle(hChildWnd);

            if(lpszName)
            {// fixed control
                char szKeyName[64];

                switch(uBtnId)
                {// button
                    case IDOK:       BgSkin_P_RegisterButtonSkin(uBtnId, "BTNSTART"); break;
                    case IDCANCEL:   BgSkin_P_RegisterButtonSkin(uBtnId, "BTNCLOSE"); break;
                }

                snprintf(szKeyName, __ARRAYSIZE(szKeyName), "%s.X", lpszName); nX = ConfigGetInt(szKeyName);
                snprintf(szKeyName, __ARRAYSIZE(szKeyName), "%s.Y", lpszName); nY = ConfigGetInt(szKeyName);
                snprintf(szKeyName, __ARRAYSIZE(szKeyName), "%s.W", lpszName); nW = ConfigGetInt(szKeyName);
                snprintf(szKeyName, __ARRAYSIZE(szKeyName), "%s.H", lpszName); nH = ConfigGetInt(szKeyName);

                if((hbmLook = BgSkin_P_GetSkin(uBtnId))!=NULL && GetObject(hbmLook, sizeof(bmBG), &bmBG))
                {// take W/H from skin in pixels
                    nW = bmBG.bmWidth/3;
                    nH = bmBG.bmHeight;

                    // switch button to use skins
                    SetWindowLongPtr(hChildWnd, GWL_STYLE, GetWindowLongPtr(hChildWnd, GWL_STYLE)|BS_OWNERDRAW);
                }

                if(GetClassKind(hChildWnd)==CTL_BASE_EDIT)
                {
                    // enable transparency
                    if(ConfigGetInt("EditBackground")==1)
                    {
                        SetWindowLongPtr(hChildWnd, GWL_EXSTYLE, GetWindowLongPtr(hChildWnd, GWL_EXSTYLE)|WS_EX_TRANSPARENT);
                    }

                    // remove border, if any
                    if(ConfigGetInt("EditFrame"))
                    {
                        SetWindowLongPtr(hChildWnd, GWL_STYLE, GetWindowLongPtr(hChildWnd, GWL_STYLE)&~WS_BORDER);
                        SetWindowLongPtr(hChildWnd, GWL_EXSTYLE, GetWindowLongPtr(hChildWnd, GWL_EXSTYLE)&~WS_EX_CLIENTEDGE);
                    }
                }

                // set size
                SetWindowPos(hChildWnd, NULL, nX, nY, nW, nH, SWP_NOZORDER);
                continue;
            }

            lpszName = ButtonGetName(uBtnId, szBuffer, __ARRAYSIZE(szBuffer));

            if(lpszName)
            {// custom button
                BgSkin_P_RegisterButtonSkin(uBtnId, lpszName);

                if((hbmLook = BgSkin_P_GetSkin(uBtnId))!=NULL && GetObject(hbmLook, sizeof(bmBG), &bmBG))
                {// custom buttons change if they are skinned
                    char szSectionName[MAX_PATH];

                    snprintf(szSectionName, __ARRAYSIZE(szSectionName), "ROCred.Buttons.%s", lpszName);  // FIXME: only button.c should (have to) know this!

                    nX = ConfigGetIntFromSection(szSectionName, "X");
                    nY = ConfigGetIntFromSection(szSectionName, "Y");
                    nW = bmBG.bmWidth/3;
                    nH = bmBG.bmHeight;

                    // switch button to use skins
                    SetWindowLongPtr(hChildWnd, GWL_STYLE, GetWindowLongPtr(hChildWnd, GWL_STYLE)|BS_OWNERDRAW);

                    // set size
                    SetWindowPos(hChildWnd, NULL, nX, nY, nW, nH, SWP_NOZORDER);
                }
                continue;
            }

            // anything else
        }
    }

    return true;
}

static void __WDECL BgSkin_P_ReleaseSkin(LPBVLLISTNODE Node, void* lpContext)
{
    LPBUTTONSKININFO lpBsi = BvLListNodeParent(Node, BUTTONSKININFO, Node);

    if(lpBsi->hbmLook!=NULL)
    {
        DeleteBitmap(lpBsi->hbmLook);
    }

    MemTFree(&lpBsi);
}

void __WDECL BgSkinFree(void)
{
    BvLListClear(&l_SkinList, &BgSkin_P_ReleaseSkin, NULL);

    if(l_hbrEditBack!=NULL)
    {
        DeleteBrush(l_hbrEditBack);
        l_hbrEditBack = NULL;
    }

    if(l_hbmBackground!=NULL)
    {
        DeleteBitmap(l_hbmBackground);
        l_hbmBackground = NULL;
    }
}
