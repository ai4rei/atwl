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

typedef bool (__stdcall* LPFNFOREACHSECTION)(const char* lpszSection, void* lpContext);

void __stdcall ConfigForEachSectionMatch(const char* lpszMatch, LPFNFOREACHSECTION Func, void* lpContext);
void __stdcall ConfigSetStr(const char* lpszKey, const char* lpszValue);
void __stdcall ConfigSetInt(const char* lpszKey, int nValue);
void __stdcall ConfigSetIntU(const char* lpszKey, unsigned int uValue);
const char* __stdcall ConfigGetStrFromSection(const char* lpszSection, const char* lpszKey);
int __stdcall ConfigGetIntFromSection(const char* lpszSection, const char* lpszKey);
unsigned int __stdcall ConfigGetIntUFromSection(const char* lpszSection, const char* lpszKey);
const char* __stdcall ConfigGetStr(const char* lpszKey);
int __stdcall ConfigGetInt(const char* lpszKey);
unsigned int __stdcall ConfigGetIntU(const char* lpszKey);
bool __stdcall ConfigInit(void);
void __stdcall ConfigQuit(void);

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif  /* CONFIG_H */
