// -----------------------------------------------------------------
// RO Credentials (ROCred)
// (c) 2012+ Ai4rei/AN
//
// -----------------------------------------------------------------

#ifndef _BUTTON_H_
#define _BUTTON_H_

bool __stdcall ButtonAction(HWND hWnd, unsigned int uBtnId);
bool __stdcall ButtonCheckName(const char* lpszName);
const char* __stdcall ButtonGetName(unsigned int uBtnId);
unsigned int __stdcall ButtonGetId(const char* lpszName);
void __stdcall ButtonFree(void);

#endif  /* _BUTTON_H_ */
