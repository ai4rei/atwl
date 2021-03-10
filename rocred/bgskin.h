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

bool __WDECL BgSkinOnEraseBkGnd(HWND const hWnd, HDC const hDC);
bool __WDECL BgSkinOnLButtonDown(HWND const hWnd);
HBRUSH __WDECL BgSkinOnCtlColorStatic(HDC const hDC, HWND const hWnd);
HBRUSH __WDECL BgSkinOnCtlColorEdit(HDC const hDC, HWND const hWnd);
bool __WDECL BgSkinOnDrawItem(UINT const uID, DRAWITEMSTRUCT const* const lpDis);
bool __WDECL BgSkinInit(HWND const hWnd);
void __WDECL BgSkinFree(void);

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif  /* BGSKIN_H */
