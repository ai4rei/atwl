// -----------------------------------------------------------------
// RO Credentials (ROCred)
// (c) 2012+ Ai4rei/AN
//
// -----------------------------------------------------------------

#ifndef BUTTON_H
#define BUTTON_H

#ifdef __cplusplus
extern "C"
{
#endif  /* __cplusplus */

bool __WDECL ButtonCreate(HWND const hWndParent, int const nX, int const nY, int const nWidth, int const nHeight, char const* const lpszDisplayName, char const* const lpszName, int const nActionType, char const* const lpszActionData, char const* const lpszActionHandler);
bool __WDECL ButtonAction(HWND const hWnd, UINT const uBtnId);
bool __WDECL ButtonCheckName(char const* const lpszName);
char const* __WDECL ButtonGetName(UINT const uBtnId, char* const lpszBuffer, size_t const uBufferSize);
UINT __WDECL ButtonGetId(char const* const lpszName);
HWND __WDECL ButtonGetDefault(void);

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif  /* BUTTON_H */
