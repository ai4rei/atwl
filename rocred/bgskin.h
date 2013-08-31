// -----------------------------------------------------------------
// RO Credentials (ROCred)
// (c) 2012-2013 Ai4rei/AN
//
// -----------------------------------------------------------------

#ifndef _BGSKIN_H_
#define _BGSKIN_H_

#include <windows.h>

void __stdcall BgSkinOnCreate(HWND hWnd);
bool __stdcall BgSkinOnEraseBkGnd(HWND hWnd, HDC hDC);
bool __stdcall BgSkinOnLButtonDown(HWND hWnd);
BOOL __stdcall BgSkinOnCtlColorStatic(HDC hDC);
bool __stdcall BgSkinOnDrawItem(UINT uID, LPDRAWITEMSTRUCT lpDis);
bool __stdcall BgSkinInit(void);
void __stdcall BgSkinQuit(void);

#endif  /* _BGSKIN_H_ */
