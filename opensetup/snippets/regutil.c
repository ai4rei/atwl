// -----------------------------------------------------------------
// NOTE: this is trimmed-down version with dependencies only
// Must not be used with other software than RO Open Setup.
// (c) 2009-2010 Ai4rei/AN
// -----------------------------------------------------------------

#include "btypes.h"
#include "regutil.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#if (ERROR_SUCCESS!=0)
    #error ERROR_SUCCESS is not zero.
#endif

bool __stdcall RegUtilLoad(void* hKey, const struct RegUtilLoadInfo* lpLi, unsigned long luElements)
{
    unsigned long i, r = 0, luLen, luType;

    for(i = 0; i<luElements; i++)
    {
        luLen = lpLi[i].luValueSize; 

        r+= RegQueryValueEx((HKEY)hKey, lpLi[i].lpszValueName, 0, &luType, lpLi[i].lpValue, &luLen);

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

bool __stdcall RegUtilSave(void* hKey, const struct RegUtilSaveInfo* lpSi, unsigned long luElements)
{
    unsigned long i, r = 0;

    for(i = 0; i<luElements; i++)
    {
        r+= RegSetValueEx((HKEY)hKey, lpSi[i].lpszValueName, 0, lpSi[i].luValueType, lpSi[i].lpValue, lpSi[i].luValueLength);
    }
    if(r)
    {
        return false;
    }
    return true;
}
