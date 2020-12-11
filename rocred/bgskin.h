// -----------------------------------------------------------------
// RO Credentials (ROCred)
// (c) 2012+ Ai4rei/AN
//
// -----------------------------------------------------------------

#ifndef BGSKIN_H
#define BGSKIN_H

#ifdef __cplusplus
extern "C"
{
#endif  /* __cplusplus */

bool __stdcall BgSkinOnEraseBkGnd(HWND hWnd, HDC hDC);
bool __stdcall BgSkinOnLButtonDown(HWND hWnd);
HBRUSH __stdcall BgSkinOnCtlColorStatic(HDC hDC, HWND hWnd);
HBRUSH __stdcall BgSkinOnCtlColorEdit(HDC hDC, HWND hWnd);
bool __stdcall BgSkinOnDrawItem(UINT uID, const DRAWITEMSTRUCT* lpDis);
bool __stdcall BgSkinInit(HWND hWnd);
void __stdcall BgSkinFree(void);

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif  /* BGSKIN_H */
