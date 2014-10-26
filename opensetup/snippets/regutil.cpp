// -----------------------------------------------------------------
// RagnarokOnline OpenSetup
// (c) 2010+ Ai4rei/AN
// See doc/license.txt for details.
// Must not be used with other software than RO Open Setup.
//
// -----------------------------------------------------------------

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "regutil.h"

bool __stdcall RegUtilLoad(HKEY hKey, LPCREGUTILLOADINFO lpLi, unsigned long luElements, long* lplLastError)
{
    long lResult, lLastError = ERROR_SUCCESS;
    unsigned long luIdx, luLen, luType;

    for(luIdx = 0; luIdx<luElements; luIdx++)
    {
        luLen = lpLi[luIdx].luValueSize; 

        lResult = RegQueryValueEx(hKey, lpLi[luIdx].lpszValueName, 0, &luType, (unsigned char*)lpLi[luIdx].lpValue, &luLen);

        if(lResult!=ERROR_SUCCESS)
        {
            lLastError = lResult;

            if(lpLi[luIdx].lpluValueLength)
            {
                lpLi[luIdx].lpluValueLength[0] = luLen;
            }
        }
        else
        {
            // different value type
            if(lpLi[luIdx].luExpectedValueType!=luType)
            {
                ZeroMemory(lpLi[luIdx].lpValue, lpLi[luIdx].luValueSize);
                continue;
            }

            // length info
            if(lpLi[luIdx].lpluValueLength)
            {
                lpLi[luIdx].lpluValueLength[0] = luLen;
            }
        }
    }

    if(lplLastError)
    {
        lplLastError[0] = lLastError;
    }

    if(lLastError!=ERROR_SUCCESS)
    {
        return false;
    }

    return true;
}

bool __stdcall RegUtilSave(HKEY hKey, LPCREGUTILSAVEINFO lpSi, unsigned long luElements, long* lplLastError)
{
    long lResult, lLastError = ERROR_SUCCESS;
    unsigned long luIdx;

    for(luIdx = 0; luIdx<luElements; luIdx++)
    {
        lResult = RegSetValueEx(hKey, lpSi[luIdx].lpszValueName, 0, lpSi[luIdx].luValueType, (const unsigned char*)lpSi[luIdx].lpValue, lpSi[luIdx].luValueLength);

        if(lResult!=ERROR_SUCCESS)
        {
            lLastError = lResult;
        }
    }

    if(lplLastError)
    {
        lplLastError[0] = lLastError;
    }

    if(lLastError!=ERROR_SUCCESS)
    {
        return false;
    }

    return true;
}

void __stdcall RegUtilDrop(HKEY hKey, const char* lpszSubKey)
{
    HKEY hSubKey;

    // attempt to delete it as a whole first, Windows 9x allows this
    // even when there are subkeys, so we can leave all the work to
    // the system.
    if(RegDeleteKey(hKey, lpszSubKey)==ERROR_SUCCESS)
    {
        return;
    }

    // slow motion for Windows NT platforms.
    if(RegOpenKeyExA(hKey, lpszSubKey, 0, KEY_READ|KEY_WRITE, &hSubKey)==ERROR_SUCCESS)
    {
        char szKeyName[MAX_PATH];
        DWORD dwKeyNameSize;
        FILETIME LastWrite;

        for(;;)
        {
            dwKeyNameSize = sizeof(szKeyName)/sizeof(szKeyName[0]);

            if(RegEnumKeyExA(hSubKey, 0, szKeyName, &dwKeyNameSize, NULL, NULL, NULL, &LastWrite)!=ERROR_SUCCESS)
            {// assume done
                break;
            }

            RegUtilDrop(hSubKey, szKeyName);
        }

        RegCloseKey(hSubKey);

        // delete for sure now
        RegDeleteKey(hKey, lpszSubKey);
    }
}
