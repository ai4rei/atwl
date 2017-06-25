// -----------------------------------------------------------------
// RO Credentials (ROCred)
// (c) 2012+ Ai4rei/AN
//
// -----------------------------------------------------------------

#ifndef _BUTTON_H_
#define _BUTTON_H_

bool __stdcall ButtonCreate(HWND hWndParent, const int nX, const int nY, const int nWidth, const int nHeight, const char* const lpszDisplayName, const char* const lpszName, const int nActionType, const char* const lpszActionData, const char* const lpszActionHandler);
bool __stdcall ButtonAction(HWND hWnd, const unsigned int uBtnId);
bool __stdcall ButtonCheckName(const char* const lpszName);
const char* __stdcall ButtonGetName(const unsigned int uBtnId, char* const lpszBuffer, const size_t uBufferSize);
unsigned int __stdcall ButtonGetId(const char* const lpszName);

#endif  /* _BUTTON_H_ */
