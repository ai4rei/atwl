// -----------------------------------------------------------------
// RagnarokOnline OpenSetup
// (c) 2010-2013 Ai4rei/AN
// See doc/license.txt for details.
//
// -----------------------------------------------------------------

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <stdio.h>
#include <string.h>

#include "opensetup.h"
#include "error.h"
#include "resource.h"

void __stdcall CError::ErrorMessageFromStringImpl(HWND hWnd, DWORD dwLastError, unsigned int uTitle, const char* lpszStrError, va_list vlArgs)
{
    char szStrTitle[256];
    char szFmtError[2048];
    char szStrSystem[256];
    char szFmtSystem[2048];
    char szFinal[4096];
    HINSTANCE hInstance = GetModuleHandle(NULL);

    // load string constants
    LoadStringA(hInstance, uTitle, szStrTitle, __ARRAYSIZE(szStrTitle));
    LoadStringA(hInstance, TEXT_ERROR__SYSTEM_ERROR, szStrSystem, __ARRAYSIZE(szStrSystem));

    // make up specific error message
    vsnprintf(szFmtError, __ARRAYSIZE(szFmtError), lpszStrError, vlArgs);

    // make up system error message
    FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 0, dwLastError, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), szFmtSystem, __ARRAYSIZE(szFmtSystem), NULL);

    // build final message
    snprintf(szFinal, __ARRAYSIZE(szFinal), "%s\r\n\r\n%s: %s", szFmtError, szStrSystem, szFmtSystem);

    MessageBox(hWnd, szFinal, szStrTitle, MB_OK|MB_ICONSTOP);
}

void __stdcall CError::ErrorMessageImpl(HWND hWnd, DWORD dwLastError, unsigned int uTitle, unsigned int uError, va_list vlArgs)
{
    char szStrError[256];
    HINSTANCE hInstance = GetModuleHandle(NULL);

    // load string constant
    LoadStringA(hInstance, uError, szStrError, __ARRAYSIZE(szStrError));

    // pass on
    CError::ErrorMessageFromStringImpl(hWnd, dwLastError, uTitle, szStrError, vlArgs);
}

void __cdecl CError::ErrorMessageFromStringEx(HWND hWnd, unsigned int uTitle, const char* lpszError, ...)
{
    va_list vlArgs;
    DWORD dwLastError = GetLastError();

    va_start(vlArgs, lpszError);
    CError::ErrorMessageFromStringImpl(hWnd, dwLastError, uTitle, lpszError, vlArgs);
    va_end(vlArgs);
}

void __cdecl CError::ErrorMessageEx(HWND hWnd, unsigned int uTitle, unsigned int uError, ...)
{
    va_list vlArgs;
    DWORD dwLastError = GetLastError();

    va_start(vlArgs, uError);
    CError::ErrorMessageImpl(hWnd, dwLastError, uTitle, uError, vlArgs);
    va_end(vlArgs);
}

void __cdecl CError::ErrorMessage(HWND hWnd, unsigned int uError, ...)
{
    va_list vlArgs;
    DWORD dwLastError = GetLastError();

    va_start(vlArgs, uError);
    CError::ErrorMessageImpl(hWnd, dwLastError, TEXT_ERROR__TITLE, uError, vlArgs);
    va_end(vlArgs);
}
