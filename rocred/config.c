// -----------------------------------------------------------------
// RO Credentials (ROCred)
// (c) 2012-2013 Ai4rei/AN
//
// -----------------------------------------------------------------

#include <windows.h>

#include <btypes.h>

#include "config.h"
#include "rocred.h"

typedef enum CONFIGDEFAULTVALUETYPE
{
    CDVT_STR,
    CDVT_NUM,
}
CONFIGDEFAULTVALUETYPE;

typedef union CONFIGDEFAULTVALUEUNION
{
    const char* lpszField;
    int nField;
}
CONFIGDEFAULTVALUEUNION;
typedef const CONFIGDEFAULTVALUEUNION* LPCCONFIGDEFAULTVALUEUNION;

typedef struct CONFIGDEFAULTVALUE
{
    const char* lpszKey;
    CONFIGDEFAULTVALUETYPE nType;
    CONFIGDEFAULTVALUEUNION Value;
}
CONFIGDEFAULTVALUE;
typedef const CONFIGDEFAULTVALUE* LPCCONFIGDEFAULTVALUE;

static const CONFIGDEFAULTVALUE l_DefaultValues[] =
{
    { "CheckSave",      CDVT_NUM, FALSE   },
    { "UserName",       CDVT_STR, ""      },
    { "ExeName",        CDVT_STR, ""      },
    { "ExeType",        CDVT_STR, "1rag1" },
    { "HashMD5",        CDVT_NUM, FALSE   },
    { "SecondInstance", CDVT_NUM, FALSE   },
};

static char l_szIniFile[MAX_PATH] = { 0 };
static char l_szMbdFile[MAX_PATH] = { 0 };
static HANDLE l_hMbd;

#define CONFIG_MAIN_SECTION "ROCred"

static LPCCONFIGDEFAULTVALUEUNION __stdcall Config_P_GetDefault(const char* lpszKey, CONFIGDEFAULTVALUETYPE nType)
{
    unsigned long luIdx;
    LPCCONFIGDEFAULTVALUE lpDv;

    for(luIdx = 0; luIdx<__ARRAYSIZE(l_DefaultValues); luIdx++)
    {
        lpDv = &l_DefaultValues[luIdx];

        if(lpDv->nType!=nType)
        {
            continue;
        }

        if(lstrcmpiA(lpDv->lpszKey, lpszKey))
        {
            continue;
        }

        break;
    }

    return luIdx==__ARRAYSIZE(l_DefaultValues) ? NULL /* crash softly */ : &lpDv->Value;
}

void __stdcall ConfigSetStr(const char* lpszKey, const char* lpszValue)
{
    WritePrivateProfileStringA(CONFIG_MAIN_SECTION, lpszKey, lpszValue, l_szIniFile);
}

void __stdcall ConfigGetStr(const char* lpszKey, char* lpszBuffer, unsigned long luBufferSize)
{
    char szDefault[1024];

    lstrcpynA(szDefault, Config_P_GetDefault(lpszKey, CDVT_STR)->lpszField, __ARRAYSIZE(szDefault));

    if(l_szMbdFile[0])
    {
        GetPrivateProfileStringA(CONFIG_MAIN_SECTION, lpszKey, szDefault, szDefault, __ARRAYSIZE(szDefault), l_szMbdFile);
    }

    GetPrivateProfileStringA(CONFIG_MAIN_SECTION, lpszKey, szDefault, lpszBuffer, luBufferSize, l_szIniFile);
}

int __stdcall ConfigGetInt(const char* lpszKey)
{
    int nDefault = Config_P_GetDefault(lpszKey, CDVT_NUM)->nField;

    if(l_szMbdFile[0])
    {
        nDefault = GetPrivateProfileIntA(CONFIG_MAIN_SECTION, lpszKey, nDefault, l_szMbdFile);
    }

    return GetPrivateProfileIntA(CONFIG_MAIN_SECTION, lpszKey, nDefault, l_szIniFile);
}

bool __stdcall ConfigInit(void)
{
    unsigned long luLen, luWritten;
    HINSTANCE hInstance = GetModuleHandleA(NULL);
    HGLOBAL hConf;
    HRSRC hInfo;

    // embedded/admin configuration
    if((hInfo = FindResourceA(hInstance, ".ini", MAKEINTRESOURCE(RT_RCDATA)))!=NULL && SizeofResource(hInstance, hInfo)>=sizeof(unsigned long))
    {
        if((hConf = LoadResource(hInstance, hInfo))!=NULL)
        {
            // resource layout: <size of data>.L <data>.?
            const unsigned long* lpluData = (const unsigned long*)LockResource(hConf);

            if(lpluData[0]<=SizeofResource(hInstance, hInfo))
            {
                char szTmpPath[MAX_PATH];
                unsigned int uUniq = 0;

                GetTempPathA(__ARRAYSIZE(szTmpPath), szTmpPath);

                do
                {
                    wsprintfA(l_szMbdFile, "%s\\~rcd%04x.ini", szTmpPath, uUniq++);

                    l_hMbd = CreateFileA(l_szMbdFile, GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, CREATE_NEW, FILE_FLAG_DELETE_ON_CLOSE|FILE_FLAG_WRITE_THROUGH, NULL);
                }
                while(l_hMbd==INVALID_HANDLE_VALUE && GetLastError()==ERROR_FILE_EXISTS && uUniq<=0xFFFF);

                if(l_hMbd!=INVALID_HANDLE_VALUE)
                {
                    if(!WriteFile(l_hMbd, &lpluData[1], lpluData[0]-sizeof(unsigned long), &luWritten, NULL))
                    {
                        // clean up the evidence if we failed
                        CloseHandle(l_hMbd);
                        l_szMbdFile[0] = 0;
                    }
                }
                else
                {
                    l_szMbdFile[0] = 0;
                }
            }
        }

        if(!l_szMbdFile[0])
        {
            // something bad happened
            return false;
        }
    }

    // external/user configuration
    GetModuleFileNameA(NULL, l_szIniFile, __ARRAYSIZE(l_szIniFile));
    luLen = lstrlenA(l_szIniFile);
    lstrcpyA((luLen>4 && l_szIniFile[luLen-4]=='.') ? &l_szIniFile[luLen-4] : &l_szIniFile[luLen], ".ini");

    return true;
}

void __stdcall ConfigQuit(void)
{
    l_szIniFile[0] = 0;

    if(l_szMbdFile[0])
    {
        CloseHandle(l_hMbd);
        l_szMbdFile[0] = 0;
    }
}
