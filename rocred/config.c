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

#define NUM(x) ((const char*)x)

static const CONFIGDEFAULTVALUE l_DefaultValues[] =
{
    { "CheckSave",                  CDVT_NUM, FALSE   },
    { "UserName",                   CDVT_STR, ""      },
    { "ExeName",                    CDVT_STR, ""      },
    { "ExeType",                    CDVT_STR, "1rag1" },
    { "HashMD5",                    CDVT_NUM, FALSE   },
    { "SecondInstance",             CDVT_NUM, FALSE   },
    { "PolicyNoCheckSave",          CDVT_NUM, FALSE   },
    { "PolicyNoReplay",             CDVT_NUM, FALSE   },
    { "PolicyNoTrayIcon",           CDVT_NUM, FALSE   },
    { "PolicyNoSessionPassword",    CDVT_NUM, FALSE   },
    { "FontSize",                   CDVT_NUM, NUM(9)  },
    { "MiscInfo",                   CDVT_NUM, 0       },
    { "_MiscInfoLastAgreed",        CDVT_NUM, 0       },
    { "IDS_USERNAME.X",             CDVT_NUM, 0       },
    { "IDS_USERNAME.Y",             CDVT_NUM, 0       },
    { "IDS_USERNAME.W",             CDVT_NUM, 0       },
    { "IDS_USERNAME.H",             CDVT_NUM, 0       },
    { "IDC_USERNAME.X",             CDVT_NUM, 0       },
    { "IDC_USERNAME.Y",             CDVT_NUM, 0       },
    { "IDC_USERNAME.W",             CDVT_NUM, 0       },
    { "IDC_USERNAME.H",             CDVT_NUM, 0       },
    { "IDS_PASSWORD.X",             CDVT_NUM, 0       },
    { "IDS_PASSWORD.Y",             CDVT_NUM, 0       },
    { "IDS_PASSWORD.W",             CDVT_NUM, 0       },
    { "IDS_PASSWORD.H",             CDVT_NUM, 0       },
    { "IDC_PASSWORD.X",             CDVT_NUM, 0       },
    { "IDC_PASSWORD.Y",             CDVT_NUM, 0       },
    { "IDC_PASSWORD.W",             CDVT_NUM, 0       },
    { "IDC_PASSWORD.H",             CDVT_NUM, 0       },
    { "IDC_CHECKSAVE.X",            CDVT_NUM, 0       },
    { "IDC_CHECKSAVE.Y",            CDVT_NUM, 0       },
    { "IDC_CHECKSAVE.W",            CDVT_NUM, 0       },
    { "IDC_CHECKSAVE.H",            CDVT_NUM, 0       },
    { "IDB_REPLAY.X",               CDVT_NUM, 0       },
    { "IDB_REPLAY.Y",               CDVT_NUM, 0       },
    { "IDB_REPLAY.W",               CDVT_NUM, 0       },
    { "IDB_REPLAY.H",               CDVT_NUM, 0       },
    { "IDOK.X",                     CDVT_NUM, 0       },
    { "IDOK.Y",                     CDVT_NUM, 0       },
    { "IDOK.W",                     CDVT_NUM, 0       },
    { "IDOK.H",                     CDVT_NUM, 0       },
    { "IDCANCEL.X",                 CDVT_NUM, 0       },
    { "IDCANCEL.Y",                 CDVT_NUM, 0       },
    { "IDCANCEL.W",                 CDVT_NUM, 0       },
    { "IDCANCEL.H",                 CDVT_NUM, 0       },
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

void __stdcall ConfigSetInt(const char* lpszKey, int nValue)
{
    char szBuffer[16];

    wsprintfA(szBuffer, "%d", nValue);
    ConfigSetStr(lpszKey, szBuffer);
}

void __stdcall ConfigSetIntU(const char* lpszKey, unsigned int uValue)
{
    char szBuffer[16];

    wsprintfA(szBuffer, "%u", uValue);
    ConfigSetStr(lpszKey, szBuffer);
}

void __stdcall ConfigGetStr(const char* lpszKey, char* lpszBuffer, unsigned long luBufferSize)
{
    char szDefault[1024];

    lstrcpynA(szDefault, Config_P_GetDefault(lpszKey, CDVT_STR)->lpszField, __ARRAYSIZE(szDefault));

    if(l_szMbdFile[0] && lpszKey[0]!='_')
    {
        GetPrivateProfileStringA(CONFIG_MAIN_SECTION, lpszKey, szDefault, szDefault, __ARRAYSIZE(szDefault), l_szMbdFile);
    }

    GetPrivateProfileStringA(CONFIG_MAIN_SECTION, lpszKey, szDefault, lpszBuffer, luBufferSize, l_szIniFile);
}

int __stdcall ConfigGetInt(const char* lpszKey)
{
    int nDefault = Config_P_GetDefault(lpszKey, CDVT_NUM)->nField;

    if(l_szMbdFile[0] && lpszKey[0]!='_')
    {
        nDefault = GetPrivateProfileIntA(CONFIG_MAIN_SECTION, lpszKey, nDefault, l_szMbdFile);
    }

    return GetPrivateProfileIntA(CONFIG_MAIN_SECTION, lpszKey, nDefault, l_szIniFile);
}

unsigned int __stdcall ConfigGetIntU(const char* lpszKey)
{
    return ConfigGetInt(lpszKey);
}

bool __stdcall ConfigSave(void)
{
    bool bSuccess = false;
    unsigned long luLen, luRead;
    void* lpData;
    HANDLE hFile = INVALID_HANDLE_VALUE;

    // load configuration
    if((hFile = CreateFileA(l_szIniFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL))!=INVALID_HANDLE_VALUE)
    {
        luLen = GetFileSize(hFile, NULL);

        if((lpData = LocalAlloc(0, luLen))!=NULL)
        {
            if(ReadFile(hFile, lpData, luLen, &luRead, NULL) && luLen==luRead)
            {
                char szSrcName[MAX_PATH];
                char szDstName[MAX_PATH];
                HANDLE hUpdate;

                GetModuleFileNameA(NULL, szSrcName, __ARRAYSIZE(szSrcName));
                wsprintfA(szDstName, "%s.embed.exe", szSrcName);

                if(CopyFileA(szSrcName, szDstName, FALSE))
                {
                    // persist as resource
                    if((hUpdate = BeginUpdateResource(szDstName, FALSE))!=NULL)
                    {
                        if(UpdateResource(hUpdate, MAKEINTRESOURCE(RT_RCDATA), "CONFIG", 0, lpData, luLen))
                        {
                            if(EndUpdateResource(hUpdate, FALSE))
                            {
                                bSuccess = true;
                            }
                        }
                        else
                        {
                            EndUpdateResource(hUpdate, TRUE);
                        }
                    }

                    if(!bSuccess)
                    {
                        DeleteFileA(szDstName);
                    }
                }
            }

            LocalFree(lpData);
        }

        CloseHandle(hFile);
    }

    return bSuccess;
}

bool __stdcall ConfigInit(void)
{
    unsigned long luLen, luWritten;
    HINSTANCE hInstance = GetModuleHandleA(NULL);
    HGLOBAL hConf;
    HRSRC hInfo;

    // embedded/admin configuration
    if((hInfo = FindResourceA(hInstance, "CONFIG", MAKEINTRESOURCE(RT_RCDATA)))!=NULL)
    {
        if((hConf = LoadResource(hInstance, hInfo))!=NULL)
        {
            const void* lpData = LockResource(hConf);
            char szTmpPath[MAX_PATH];
            unsigned int uUniq = 0;

            GetTempPathA(__ARRAYSIZE(szTmpPath), szTmpPath);

            do
            {
                wsprintfA(l_szMbdFile, "%s\\~rcd%04x.ini", szTmpPath, uUniq++);

                // BUG: Keeping the file open for the sake of delete
                //      on close will prevent GetPrivateProfile* on
                //      Windows 9x, because of exclusive access.
                l_hMbd = CreateFileA(l_szMbdFile, GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, CREATE_NEW, FILE_FLAG_DELETE_ON_CLOSE|FILE_FLAG_WRITE_THROUGH, NULL);
            }
            while(l_hMbd==INVALID_HANDLE_VALUE && GetLastError()==ERROR_FILE_EXISTS && uUniq<=0xFFFF);

            if(l_hMbd!=INVALID_HANDLE_VALUE)
            {
                if(!WriteFile(l_hMbd, lpData, SizeofResource(hInstance, hInfo), &luWritten, NULL))
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
