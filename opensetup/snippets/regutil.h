// -----------------------------------------------------------------
// NOTE: this is trimmed-down version with dependencies only
// Must not be used with other software than RO Open Setup.
// (c) 2009-2010 Ai4rei/AN
// -----------------------------------------------------------------

#ifndef _REGUTIL_H_
#define _REGUTIL_H_

#ifdef __cplusplus
extern "C"
{
#endif

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

bool __stdcall RegUtilLoad(void* hKey, const struct RegUtilLoadInfo* lpLi, unsigned long luElements);
bool __stdcall RegUtilSave(void* hKey, const struct RegUtilSaveInfo* lpSi, unsigned long luElements);

#ifdef __cplusplus
};
#endif

#endif  /* _REGUTIL_H_ */
