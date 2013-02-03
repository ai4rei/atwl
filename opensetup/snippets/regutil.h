// -----------------------------------------------------------------
// RagnarokOnline OpenSetup
// (c) 2010-2013 Ai4rei/AN
// See doc/license.txt for details.
// Must not be used with other software than RO Open Setup.
//
// -----------------------------------------------------------------

#ifndef _REGUTIL_H_
#define _REGUTIL_H_

struct RegUtilLoadInfo
{
    const char* lpszValueName;
    void* lpValue;
    unsigned long luValueSize;
    unsigned long* lpluValueLength;
    unsigned long luExpectedValueType;
};

struct RegUtilSaveInfo
{
    const char* lpszValueName;
    void* lpValue;
    unsigned long luValueLength;
    unsigned long luValueType;
};

bool __stdcall RegUtilLoad(HKEY hKey, const struct RegUtilLoadInfo* lpLi, unsigned long luElements);
bool __stdcall RegUtilSave(HKEY hKey, const struct RegUtilSaveInfo* lpSi, unsigned long luElements);

#endif  /* _REGUTIL_H_ */
