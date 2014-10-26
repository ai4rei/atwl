// -----------------------------------------------------------------
// RagnarokOnline OpenSetup
// (c) 2010+ Ai4rei/AN
// See doc/license.txt for details.
//
// -----------------------------------------------------------------

#include <windows.h>

bool __stdcall VersionInfoMatch(const char* lpszFileName, const char* lpszStrName, const char* lpszMatch, bool bCaseSensitive)
{
    bool bMatched = false;
    DWORD dwDummy, dwSize = GetFileVersionInfoSizeA(lpszFileName, &dwDummy);

    if(dwSize)
    {
        LPVOID lpInfo = GlobalAlloc(GMEM_FIXED, dwSize);

        if(lpInfo)
        {
            if(GetFileVersionInfoA(lpszFileName, dwDummy, dwSize, lpInfo))
            {
                UINT uLangSize;
                LPDWORD lpdwLang;

                if(VerQueryValueA(lpInfo, "\\VarFileInfo\\Translation", (LPVOID*)&lpdwLang, &uLangSize))
                {
                    UINT uIdx;

                    for(uIdx = 0; uIdx<uLangSize/sizeof(lpdwLang[0]); uIdx++)
                    {
                        char szKeyName[256];
                        const char* lpszValue;
                        UINT uValueSize;

                        wsprintfA(szKeyName, "\\StringFileInfo\\%04x%04x\\%s", LOWORD(lpdwLang[uIdx]), HIWORD(lpdwLang[uIdx]), lpszStrName);

                        if(VerQueryValue(lpInfo, szKeyName, (LPVOID*)&lpszValue, &uValueSize))
                        {
                            if(!(bCaseSensitive ? lstrcmpA : lstrcmpiA)(lpszValue, lpszMatch))
                            {
                                bMatched = true;
                                break;
                            }
                        }
                    }
                }
                else
                {/* no translation table, assume */
                    char szKeyName[256];
                    const char* lpszValue;
                    UINT uValueSize;

                    wsprintfA(szKeyName, "\\StringFileInfo\\040904E4\\%s", lpszStrName);

                    if(VerQueryValue(lpInfo, szKeyName, (LPVOID*)&lpszValue, &uValueSize))
                    {
                        if(!(bCaseSensitive ? lstrcmpA : lstrcmpiA)(lpszValue, lpszMatch))
                        {
                            bMatched = true;
                        }
                    }
                }
            }

            GlobalFree(lpInfo);
        }
    }

    return bMatched;
}
