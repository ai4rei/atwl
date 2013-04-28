// -----------------------------------------------------------------
// RagnarokOnline OpenSetup
// (c) 2010-2013 Ai4rei/AN
// See doc/license.txt for details.
// Must not be used with other software than RO Open Setup.
//
// -----------------------------------------------------------------

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "regutil.h"

#if (ERROR_SUCCESS!=0)
    #error ERROR_SUCCESS is not zero.
#endif

bool __stdcall RegUtilLoad(HKEY hKey, LPCREGUTILLOADINFO lpLi, unsigned long luElements)
{
    unsigned long i, r = 0, luLen, luType;

    for(i = 0; i<luElements; i++)
    {
        luLen = lpLi[i].luValueSize; 

        r+= RegQueryValueEx(hKey, lpLi[i].lpszValueName, 0, &luType, (unsigned char*)lpLi[i].lpValue, &luLen);

        if(lpLi[i].luExpectedValueType!=luType)
        {
            ZeroMemory(lpLi[i].lpValue, lpLi[i].luValueSize);
            continue;
        }

        if(lpLi[i].lpluValueLength)
        {
            lpLi[i].lpluValueLength[0] = luLen;
        }
    }

    if(r)
    {
        return false;
    }

    return true;
}

bool __stdcall RegUtilSave(HKEY hKey, LPCREGUTILSAVEINFO lpSi, unsigned long luElements)
{
    unsigned long i, r = 0;

    for(i = 0; i<luElements; i++)
    {
        r+= RegSetValueEx(hKey, lpSi[i].lpszValueName, 0, lpSi[i].luValueType, (const unsigned char*)lpSi[i].lpValue, lpSi[i].luValueLength);
    }

    if(r)
    {
        return false;
    }

    return true;
}
