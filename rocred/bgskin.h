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

bool __WDECL BgSkinOnEraseBkGnd(HWND hWnd, HDC hDC);
bool __WDECL BgSkinOnLButtonDown(HWND hWnd);
HBRUSH __WDECL BgSkinOnCtlColorStatic(HDC hDC, HWND hWnd);
HBRUSH __WDECL BgSkinOnCtlColorEdit(HDC hDC, HWND hWnd);
bool __WDECL BgSkinOnDrawItem(UINT uID, const DRAWITEMSTRUCT* lpDis);
bool __WDECL BgSkinInit(HWND hWnd);
void __WDECL BgSkinFree(void);

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif  /* BGSKIN_H */
