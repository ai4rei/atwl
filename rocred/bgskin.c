// -----------------------------------------------------------------
// RO Credentials (ROCred)
// (c) 2012-2013 Ai4rei/AN
//
// -----------------------------------------------------------------

#include <windows.h>

#include <btypes.h>
#include <regionui.h>

#include "bgskin.h"
#include "config.h"
#include "rocred.h"

// uxtheme.dll specific
#ifndef WM_THEMECHANGED
    #define WM_THEMECHANGED 0x031A
#endif  /* WM_THEMECHANGED */
typedef void (WINAPI* LPFNSETTHEMEAPPPROPERTIES)(DWORD dwFlags);
// end uxtheme.dll

typedef struct BGSKINBATCHINFO
{
    UINT uID;
    const char* lpszName;
}
BGSKINBATCHINFO,* LPBGSKINBATCHINFO;
typedef const BGSKINBATCHINFO* LPCBGSKINBATCHINFO;

typedef struct BUTTONSKININFO
{
    UINT uID;
    const char* lpszName;
    HBITMAP hbmLook;
}
BUTTONSKININFO,* LPBUTTONSKININFO;
typedef const BUTTONSKININFO* LPCBUTTONSKININFO;

#define BGSKINBATCHINIT(x) { x, #x }
#define BUTTONSKININIT(x,name) { x, name }

static const BGSKINBATCHINFO l_BatchInfo[] =
{
    BGSKINBATCHINIT(IDS_USERNAME),
    BGSKINBATCHINIT(IDC_USERNAME),
    BGSKINBATCHINIT(IDS_PASSWORD),
    BGSKINBATCHINIT(IDC_PASSWORD),
    BGSKINBATCHINIT(IDC_CHECKSAVE),
    BGSKINBATCHINIT(IDB_REPLAY),
    BGSKINBATCHINIT(IDOK),
    BGSKINBATCHINIT(IDCANCEL),
};
static BUTTONSKININFO l_ButtonSkinInfo[] =
{
    BUTTONSKININIT(IDB_REPLAY, "btnreplay"),
    BUTTONSKININIT(IDOK, "btnstart"),
    BUTTONSKININIT(IDCANCEL, "btnclose"),
};
static HBITMAP l_hbmBackground = NULL;

static bool __stdcall BgSkin_P_QueryButtonSkinInfo(UINT uID, LPBUTTONSKININFO* lppBsi)
{
    unsigned long luIdx;

    for(luIdx = 0; luIdx<__ARRAYSIZE(l_ButtonSkinInfo); luIdx++)
    {
        if(l_ButtonSkinInfo[luIdx].uID==uID)
        {
            lppBsi[0] = &l_ButtonSkinInfo[luIdx];

            return true;
        }
    }

    lppBsi[0] = NULL;

    return false;
}

static HBITMAP __stdcall BgSkin_P_LoadBitmap(const char* lpszFileName, const char* lpszImageName)
{
    HBITMAP hBitmap;

    // try local file
    if((hBitmap = LoadImage(NULL, lpszFileName, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE))==NULL)
    {
        // fall back to embedded, but do not actually care about the result
        hBitmap = LoadImage(GetModuleHandle(NULL), lpszImageName, IMAGE_BITMAP, 0, 0, 0);
    }

    return hBitmap;
}

static bool __stdcall BgSkin_P_IsActive(void)
{
    return (bool)(l_hbmBackground!=NULL);
}

static void __stdcall BgSkin_P_DisableVisualStyles(HWND hWnd)
{
    LPFNSETTHEMEAPPPROPERTIES SetThemeAppProperties = NULL;
    HMODULE hDll = LoadLibraryA("uxtheme.dll");

    if(hDll)
    {
        if((SetThemeAppProperties = (LPFNSETTHEMEAPPPROPERTIES)GetProcAddress(hDll, "SetThemeAppProperties"))!=NULL)
        {
            // disable visual styles
            SetThemeAppProperties(0x1 /* STAP_ALLOW_NONCLIENT */);
            SendMessage(hWnd, WM_THEMECHANGED, 0, 0);
        }

        FreeLibrary(hDll);
    }
}

void __stdcall BgSkinOnCreate(HWND hWnd)
{
    char szName[32];
    int nX, nY, nW, nH;
    unsigned long luIdx;
    BITMAP bmBG;
    HRGN hRGN;
    RECT rcWnd, rcWA;

    if(BgSkin_P_IsActive())
    {
        if(GetObject(l_hbmBackground, sizeof(bmBG), &bmBG))
        {
            // set window size to bitmap size and get rid of
            // non-client elements while being at it
            SetWindowLongPtr(hWnd, GWL_STYLE, WS_POPUP|WS_SYSMENU|WS_MINIMIZEBOX);
            SetWindowPos(hWnd, NULL, 0, 0, bmBG.bmWidth, bmBG.bmHeight, SWP_NOZORDER|SWP_NOMOVE);

            // center window
            GetWindowRect(hWnd, &rcWnd);
            SystemParametersInfo(SPI_GETWORKAREA, 0, &rcWA, 0);
            OffsetRect(&rcWA, ((rcWA.right-rcWA.left)-(rcWnd.right-rcWnd.left))/2, ((rcWA.bottom-rcWA.top)-(rcWnd.bottom-rcWnd.top))/2);
            SetWindowPos(hWnd, NULL, rcWA.left, rcWA.top, 0, 0, SWP_NOSIZE|SWP_NOZORDER);

            // window size for base window region (origin transform)
            GetWindowRect(hWnd, &rcWnd);
            rcWnd.right-= rcWnd.left;
            rcWnd.bottom-= rcWnd.top;
            rcWnd.left = rcWnd.top = 0;

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
                    DeleteObject(hRGN);
                }
            }
        }

        // load child control metrics
        for(luIdx = 0; luIdx<__ARRAYSIZE(l_BatchInfo); luIdx++)
        {
            LPBUTTONSKININFO lpBsi;
            LPCBGSKINBATCHINFO lpBbi = &l_BatchInfo[luIdx];
            HWND hChild = GetDlgItem(hWnd, lpBbi->uID);

            wsprintfA(szName, "%s.X", lpBbi->lpszName);
            nX = ConfigGetInt(szName);

            wsprintfA(szName, "%s.Y", lpBbi->lpszName);
            nY = ConfigGetInt(szName);

            wsprintfA(szName, "%s.W", lpBbi->lpszName);
            nW = ConfigGetInt(szName);

            wsprintfA(szName, "%s.H", lpBbi->lpszName);
            nH = ConfigGetInt(szName);

            SetWindowPos(hChild, NULL, nX, nY, nW, nH, SWP_NOOWNERZORDER|SWP_NOZORDER);

            // disable client area visual styles to prevent glitches
            BgSkin_P_DisableVisualStyles(hChild);

            // is this a button? load skin for it, too!
            if(BgSkin_P_QueryButtonSkinInfo(lpBbi->uID, &lpBsi))
            {
                char szFileName[64];

                wsprintfA(szFileName, "%s.bmp", lpBsi->lpszName);

                if((lpBsi->hbmLook = BgSkin_P_LoadBitmap(szFileName, lpBbi->lpszName))!=NULL && GetObject(lpBsi->hbmLook, sizeof(bmBG), &bmBG))
                {
                    // switch button to use skins
                    SetWindowLongPtr(hChild, GWL_STYLE, GetWindowLongPtr(hChild, GWL_STYLE)|BS_OWNERDRAW);

                    // set size to the bitmap
                    SetWindowPos(hChild, NULL, 0, 0, bmBG.bmWidth, bmBG.bmHeight, SWP_NOZORDER|SWP_NOMOVE);
                }
            }
        }
    }
}

bool __stdcall BgSkinOnEraseBkGnd(HWND hWnd, HDC hDC)
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

bool __stdcall BgSkinOnLButtonDown(HWND hWnd)
{
    if(BgSkin_P_IsActive())
    {
        // enable moving everywhere, since we no longer have a caption
        SendMessage(hWnd, WM_NCLBUTTONDOWN, HTCAPTION, 0);
        return true;
    }

    return false;
}

BOOL __stdcall BgSkinOnCtlColorStatic(HDC hDC)
{
    if(BgSkin_P_IsActive())
    {
        SetBkMode(hDC, TRANSPARENT);

        return (BOOL)GetStockObject(NULL_BRUSH);
    }

    return FALSE;
}

bool __stdcall BgSkinOnDrawItem(UINT uID, LPDRAWITEMSTRUCT lpDis)
{
    HDC hdcBitmap;
    LPBUTTONSKININFO lpBsi;

    if(BgSkin_P_QueryButtonSkinInfo(uID, &lpBsi) && lpBsi->hbmLook)
    {
        if((hdcBitmap = CreateCompatibleDC(lpDis->hDC))!=NULL)
        {
            HGDIOBJ hGdiObj = SelectObject(hdcBitmap, lpBsi->hbmLook);

            BitBlt(lpDis->hDC, lpDis->rcItem.left, lpDis->rcItem.top, lpDis->rcItem.right-lpDis->rcItem.left, lpDis->rcItem.bottom-lpDis->rcItem.top, hdcBitmap, lpDis->rcItem.left, lpDis->rcItem.top, SRCCOPY);

            SelectObject(hdcBitmap, hGdiObj);
            DeleteDC(hdcBitmap);
            return true;
        }
    }

    return false;
}

bool __stdcall BgSkinInit(void)
{
    l_hbmBackground = BgSkin_P_LoadBitmap("bgskin.bmp", "BGSKIN");

    return true;
}

void __stdcall BgSkinQuit(void)
{
    unsigned long luIdx;

    for(luIdx = 0; luIdx<__ARRAYSIZE(l_ButtonSkinInfo); luIdx++)
    {
        if(l_ButtonSkinInfo[luIdx].hbmLook)
        {
            DeleteObject(l_ButtonSkinInfo[luIdx].hbmLook);
            l_ButtonSkinInfo[luIdx].hbmLook = NULL;
        }
    }

    if(l_hbmBackground)
    {
        DeleteObject(l_hbmBackground);
        l_hbmBackground = NULL;
    }
}
