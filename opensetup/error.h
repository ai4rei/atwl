// -----------------------------------------------------------------
// RagnarokOnline OpenSetup
// (c) 2010-2014 Ai4rei/AN
// See doc/license.txt for details.
//
// -----------------------------------------------------------------

#ifndef _ERROR_H_
#define _ERROR_H_

class CError
{
private:
    static void __stdcall ErrorMessageFromStringImpl(HWND hWnd, DWORD dwLastError, unsigned int uTitle, const char* lpszStrError, va_list vlArgs);
    static void __stdcall ErrorMessageImpl(HWND hWnd, DWORD dwLastError, unsigned int uTitle, unsigned int uError, va_list vlArgs);

public:
    static void __cdecl ErrorMessageFromStringEx(HWND hWnd, unsigned int uTitle, const char* lpszError, ...);
    static void __cdecl ErrorMessageEx(HWND hWnd, unsigned int uTitle, unsigned int uError, ...);
    static void __cdecl ErrorMessage(HWND hWnd, unsigned int uError, ...);
};

#endif  /* _ERROR_H_ */
