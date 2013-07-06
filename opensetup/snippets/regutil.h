// -----------------------------------------------------------------
// RagnarokOnline OpenSetup
// (c) 2010-2013 Ai4rei/AN
// See doc/license.txt for details.
// Must not be used with other software than RO Open Setup.
//
// -----------------------------------------------------------------

#ifndef _REGUTIL_H_
#define _REGUTIL_H_

typedef struct REGUTILLOADINFO
{
    const char* lpszValueName;
    void* lpValue;
    unsigned long luValueSize;
    unsigned long* lpluValueLength;
    unsigned long luExpectedValueType;
}
REGUTILLOADINFO,* LPREGUTILLOADINFO;
typedef const struct REGUTILLOADINFO* LPCREGUTILLOADINFO;

typedef struct REGUTILSAVEINFO
{
    const char* lpszValueName;
    void* lpValue;
    unsigned long luValueLength;
    unsigned long luValueType;
}
REGUTILSAVEINFO,* LPREGUTILSAVEINFO;
typedef const struct REGUTILSAVEINFO* LPCREGUTILSAVEINFO;

bool __stdcall RegUtilLoad(HKEY hKey, LPCREGUTILLOADINFO lpLi, unsigned long luElements, long* lplLastError);
bool __stdcall RegUtilSave(HKEY hKey, LPCREGUTILSAVEINFO lpSi, unsigned long luElements, long* lplLastError);

void __stdcall RegUtilDrop(HKEY hKey, const char* lpszSubKey);

#endif  /* _REGUTIL_H_ */
