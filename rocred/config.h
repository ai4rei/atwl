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

typedef bool (__WDECL* LPFNFOREACHSECTION)(char const* const lpszSection, void* lpContext);

void __WDECL ConfigForEachSectionMatch(char const* const lpszMatch, LPFNFOREACHSECTION const Func, void* lpContext);
void __WDECL ConfigSetStr(char const* const lpszKey, char const* const lpszValue);
void __WDECL ConfigSetInt(char const* const lpszKey, int const nValue);
void __WDECL ConfigSetIntU(char const* const lpszKey, unsigned int const uValue);
char const* __WDECL ConfigGetStrFromSection(char const* const lpszSection, char const* const lpszKey);
int __WDECL ConfigGetIntFromSection(char const* const lpszSection, char const* const lpszKey);
unsigned int __WDECL ConfigGetIntUFromSection(char const* const lpszSection, char const* const lpszKey);
char const* __WDECL ConfigGetStr(char const* const lpszKey);
int __WDECL ConfigGetInt(char const* const lpszKey);
unsigned int __WDECL ConfigGetIntU(char const* const lpszKey);
bool __WDECL ConfigInit(void);
void __WDECL ConfigQuit(void);

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif  /* CONFIG_H */
