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

bool __WDECL ButtonCreate(HWND hWndParent, const int nX, const int nY, const int nWidth, const int nHeight, const char* const lpszDisplayName, const char* const lpszName, const int nActionType, const char* const lpszActionData, const char* const lpszActionHandler);
bool __WDECL ButtonAction(HWND hWnd, const unsigned int uBtnId);
bool __WDECL ButtonCheckName(const char* const lpszName);
const char* __WDECL ButtonGetName(const unsigned int uBtnId, char* const lpszBuffer, const size_t uBufferSize);
unsigned int __WDECL ButtonGetId(const char* const lpszName);
HWND __WDECL ButtonGetDefault(void);

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif  /* BUTTON_H */
