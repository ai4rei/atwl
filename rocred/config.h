// -----------------------------------------------------------------
// RO Credentials (ROCred)
// (c) 2012+ Ai4rei/AN
//
// -----------------------------------------------------------------

#ifndef CONFIG_H
#define CONFIG_H

#ifdef __cplusplus
extern "C"
{
#endif  /* __cplusplus */

typedef bool (__WDECL* LPFNFOREACHSECTION)(const char* lpszSection, void* lpContext);

void __WDECL ConfigForEachSectionMatch(const char* lpszMatch, LPFNFOREACHSECTION Func, void* lpContext);
void __WDECL ConfigSetStr(const char* lpszKey, const char* lpszValue);
void __WDECL ConfigSetInt(const char* lpszKey, int nValue);
void __WDECL ConfigSetIntU(const char* lpszKey, unsigned int uValue);
const char* __WDECL ConfigGetStrFromSection(const char* lpszSection, const char* lpszKey);
int __WDECL ConfigGetIntFromSection(const char* lpszSection, const char* lpszKey);
unsigned int __WDECL ConfigGetIntUFromSection(const char* lpszSection, const char* lpszKey);
const char* __WDECL ConfigGetStr(const char* lpszKey);
int __WDECL ConfigGetInt(const char* lpszKey);
unsigned int __WDECL ConfigGetIntU(const char* lpszKey);
bool __WDECL ConfigInit(void);
void __WDECL ConfigQuit(void);

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif  /* CONFIG_H */
