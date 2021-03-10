// -----------------------------------------------------------------
// RO Credentials (ROCred)
// (c) 2012+ Ai4rei/AN
//
// -----------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <btypes.h>
#include <kvdb.h>
#include <memtaf.h>
#include <w32ex.h>

#include "rocred.h"

#include "config.h"

struct LPFOREACHSECTIONCONTEXT
{
    LPFNFOREACHSECTION Func;
    void* lpContext;
};

static char l_szIniFile[MAX_PATH] = { 0 };
static KVDB l_ConfigDB = { 0 };

#define CONFIG_MAIN_SECTION "ROCred"

static bool __WDECL Config_P_ForEachSection(LPKVDB DB, LPKVDBSECTION Section, const char* const lpszSection, void* lpContext)
{
    struct LPFOREACHSECTIONCONTEXT* lpCtx = (struct LPFOREACHSECTIONCONTEXT*)lpContext;

    return lpCtx->Func(lpszSection, lpCtx->lpContext);
}

void __WDECL ConfigForEachSectionMatch(const char* lpszMatch, LPFNFOREACHSECTION Func, void* lpContext)
{
    struct LPFOREACHSECTIONCONTEXT Ctx = { Func, lpContext };

    KvForEachSectionMatch(&l_ConfigDB, lpszMatch, &Config_P_ForEachSection, &Ctx);
}

void __WDECL ConfigSetStr(const char* lpszKey, const char* lpszValue)
{
    if(lpszValue)
    {
        KvKeySetStrValue(&l_ConfigDB, NULL, CONFIG_MAIN_SECTION, NULL, lpszKey, lpszValue);
    }
    else
    {
        KvKeyDelete(&l_ConfigDB, NULL, CONFIG_MAIN_SECTION, NULL, lpszKey);
    }

    KvSave(&l_ConfigDB, l_szIniFile);
}

void __WDECL ConfigSetInt(const char* lpszKey, int nValue)
{
    char szBuffer[16];

    snprintf(szBuffer, __ARRAYSIZE(szBuffer), "%d", nValue);
    ConfigSetStr(lpszKey, szBuffer);
}

void __WDECL ConfigSetIntU(const char* lpszKey, unsigned int uValue)
{
    char szBuffer[16];

    snprintf(szBuffer, __ARRAYSIZE(szBuffer), "%u", uValue);
    ConfigSetStr(lpszKey, szBuffer);
}

const char* __WDECL ConfigGetStrFromSection(const char* lpszSection, const char* lpszKey)
{
    return KvKeyGetStrValue(&l_ConfigDB, NULL, lpszSection, NULL, lpszKey);
}

int __WDECL ConfigGetIntFromSection(const char* lpszSection, const char* lpszKey)
{
    return atoi(ConfigGetStrFromSection(lpszSection, lpszKey));
}

unsigned int __WDECL ConfigGetIntUFromSection(const char* lpszSection, const char* lpszKey)
{
    return ConfigGetIntFromSection(lpszSection, lpszKey);
}

const char* __WDECL ConfigGetStr(const char* lpszKey)
{
    return ConfigGetStrFromSection(CONFIG_MAIN_SECTION, lpszKey);
}

int __WDECL ConfigGetInt(const char* lpszKey)
{
    return ConfigGetIntFromSection(CONFIG_MAIN_SECTION, lpszKey);
}

unsigned int __WDECL ConfigGetIntU(const char* lpszKey)
{
    return ConfigGetIntUFromSection(CONFIG_MAIN_SECTION, lpszKey);
}

static bool __WDECL Config_P_FoilEachKey(LPKVDB DB, const char* const lpszSection, LPKVDBKEY Key, const char* const lpszKey, void* lpContext)
{
    if(lpszKey[0]=='_')
    {
        KvKeyDelete(NULL, NULL, NULL, Key, NULL);
    }

    return true;
}

static bool __WDECL Config_P_FoilEachSection(LPKVDB DB, LPKVDBSECTION Section, const char* const lpszSection, void* lpContext)
{
    KvForEachKey(NULL, Section, NULL, &Config_P_FoilEachKey, NULL);
    return true;
}

bool __WDECL ConfigInit(void)
{
    bool bSuccess = false;
    DWORD dwLength = 0;
    LPVOID lpData = NULL;

    // set defaults
    KvInit(&l_ConfigDB, &g_Win32PrivateProfileAdapter);
    KvKeySetStrValue(&l_ConfigDB, NULL, CONFIG_MAIN_SECTION, NULL, "ExeType", "1rag");
    KvKeySetStrValue(&l_ConfigDB, NULL, CONFIG_MAIN_SECTION, NULL, "FontSize", "9");
    KvDirty(&l_ConfigDB, false);

    // load embedded/admin configuration
    if(ResourceFetch(NULL, MAKEINTRESOURCEA(RT_RCDATA), "CONFIG", &dwLength, &lpData))
    {
        for(;;)
        {
            char szTmpPath[MAX_PATH];
            char szMbdFile[MAX_PATH];
            HANDLE hFile;

            if(!GetTempPathExA(szTmpPath, __ARRAYSIZE(szTmpPath)))
            {
                break;
            }

            if(!GetTempFileNameExA(szTmpPath, "~rcd", 0, szMbdFile, __ARRAYSIZE(szMbdFile)) || GetLastError()!=ERROR_SUCCESS)
            {
                break;
            }

            hFile = CreateFileA(szMbdFile, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_NONE, NULL, OPEN_EXISTING /* GetTempFileNameEx created it */, 0, NULL);

            if(hFile!=INVALID_HANDLE_VALUE)
            {
                DWORD dwWritten = 0;

                if(WriteFile(hFile, lpData, dwLength, &dwWritten, NULL))
                {
                    CloseFile(&hFile);

                    if(KvLoad(&l_ConfigDB, szMbdFile))
                    {
                        // foil attempts to override protected defaults
                        KvForEachSection(&l_ConfigDB, &Config_P_FoilEachSection, NULL);

                        bSuccess = true;
                    }
                }
                else
                {
                    CloseFile(&hFile);
                }
            }

            // clean up the evidence
            DeleteFileA(szMbdFile);
            break;
        }

        if(!bSuccess)
        {
            // something bad happened
            return false;
        }
    }

    // external/user configuration
    if(GetModuleFileNameSpecificPathA(NULL, l_szIniFile, __ARRAYSIZE(l_szIniFile), NULL, "ini"))
    {
        KvLoad(&l_ConfigDB, l_szIniFile);
    }

    return true;
}

void __WDECL ConfigQuit(void)
{
    KvFree(&l_ConfigDB);
}
