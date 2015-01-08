// -----------------------------------------------------------------
// RO Credentials (ROCred)
// (c) 2012-2015 Ai4rei/AN
//
// -----------------------------------------------------------------

#ifndef _BGSKIN_H_
#define _BGSKIN_H_

#include <windows.h>

bool __stdcall BgSkinOnEraseBkGnd(HWND hWnd, HDC hDC);
bool __stdcall BgSkinOnLButtonDown(HWND hWnd);
BOOL __stdcall BgSkinOnCtlColorStatic(HDC hDC, HWND hWnd);
BOOL __stdcall BgSkinOnCtlColorEdit(HDC hDC, HWND hWnd);
bool __stdcall BgSkinOnDrawItem(UINT uID, LPDRAWITEMSTRUCT lpDis);
bool __stdcall BgSkinInit(HWND hWnd);
void __stdcall BgSkinFree(void);

#endif  /* _BGSKIN_H_ */
