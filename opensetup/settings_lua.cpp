// -----------------------------------------------------------------
// RagnarokOnline OpenSetup
// (c) 2010+ Ai4rei/AN
// See doc/license.txt for details.
//
// -----------------------------------------------------------------

#include <stdio.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <regutil.h>

#include "opensetup.h"
#include "config.h"
#include "error.h"
#include "log.h"
#include "luaio.h"
#include "resource.h"
#include "settings.h"
#include "settings_lua.h"

#define SETTINGS_REGPATH "Software\\Gravity Soft\\Ragnarok"
#define SETTINGS_RENEWPATH "Software\\Gravity Soft\\RenewSetup"
#define SETTINGS_SETMASK "%s[\"%s\"] = %d\r\n"
#define SETTINGS_CAT_OPTIONLIST "OptionInfoList"
#define SETTINGS_CAT_COMMANDLIST "CmdOnOffList"

static const char* l_lppszResetFileList[] =
{
    "savedata",
    // old clients
    "data\\ChatWndInfo_U.lua",
    "data\\OptionInfo.lua",
    "data\\UserKeys_s.lua",
};

typedef struct LUAOPTIONINFOINT
{
    const char* lpszCategory;
    const char* lpszName;
    unsigned long* lpluValue;  // there is no non-number saving in LUA right now
}
LUAOPTIONINFOINT;

// not part of the class, as we do not want to introduce CLuaIO to everyone
static bool __stdcall CSettingsLua_P_ForEachKeySave(CLuaIO* This, const char* lpszTable, const char* lpszKey, int nKeyType, int nValType, void* lpContext)
{
    FILE* hFile = (FILE*)lpContext;
    char* lpszString = NULL;

    This->GetTableString(lpszTable, lpszKey, &lpszString);

    if(lpszString==NULL)
    {
        fprintf(hFile, "-- ");
    }

    fprintf(hFile, "%s[", lpszTable);

    if(nKeyType==LUA_TSTRING)
    {
        fprintf(hFile, "\"%s\"", lpszKey);
    }
    else
    {
        fprintf(hFile, "%s", lpszKey);
    }

    fprintf(hFile, "] = ");

    if(lpszString==NULL)
    {
        fprintf(hFile, "???");
    }
    else
    {
        if(nValType==LUA_TSTRING)
        {
            fprintf(hFile, "\"%s\"", lpszString);
        }
        else
        {
            fprintf(hFile, "%s", lpszString);
        }

        free(lpszString);
    }

    fprintf(hFile, "\r\n");

    return true;
}

const char* __stdcall CSettingsLua::P_GetLuaPath()
{
    int nID = g_Config.GetNumber("settings", "LuaSaveData", 1);

    switch(nID)
    {
        case 0:
            return "data";
        default:
            g_Log.LogError("LuaSaveData specifies unknown settings folder ID %d.", nID);
            // fall through
        case 1:
            return "savedata";
    }
}

const char* __stdcall CSettingsLua::P_GetLuaFile()
{
    static char szBuffer[MAX_PATH] = { 0 };

    if(!szBuffer[0])
    {
        snprintf(szBuffer, __ARRAYSIZE(szBuffer), "%s\\OptionInfo", P_GetLuaPath());
    }

    return szBuffer;
}

const char* __stdcall CSettingsLua::P_GetLuaFull()
{
    static char szBuffer[MAX_PATH] = { 0 };

    if(!szBuffer[0])
    {
        snprintf(szBuffer, __ARRAYSIZE(szBuffer), "%s.lua", P_GetLuaFile());
    }

    return szBuffer;
}

bool __stdcall CSettingsLua::Save()
{
    HKEY hKey;
    LONG lResult;
    REGUTILSAVEINFO SaveInfo[] =
    {
#define SAVEENTRY(name,type) { #name, &this->m_Entries.##name, sizeof(this->m_Entries.##name), (type) }
        SAVEENTRY(SOUNDMODE,        REG_DWORD ),
        SAVEENTRY(SPEAKERTYPE,      REG_DWORD ),
        SAVEENTRY(DIGITALRATETYPE,  REG_DWORD ),
        SAVEENTRY(DIGITALBITSTYPE,  REG_DWORD ),
        SAVEENTRY(GUIDDRIVER,       REG_BINARY),
        SAVEENTRY(GUIDDEVICE,       REG_BINARY),
        SAVEENTRY(DEVICENAME,       REG_SZ    ),
        SAVEENTRY(PROVIDERNAME,     REG_SZ    ),
        SAVEENTRY(SHOWTIPSATSTARTUP,REG_DWORD ),
#undef SAVEENTRY
    };
    LUAOPTIONINFOINT LuaSaveInfo[] =
    {
#define SAVEENTRY(name,cat,optname) { cat, optname, &this->m_Entries.##name }
        SAVEENTRY(TRILINEARFILTER,      SETTINGS_CAT_OPTIONLIST, "Trilinear"       ),
        SAVEENTRY(STREAMVOLUME,         SETTINGS_CAT_OPTIONLIST, "Bgm_Volume"      ),
        SAVEENTRY(SOUNDVOLUME,          SETTINGS_CAT_OPTIONLIST, "Effect_Volume"   ),
        SAVEENTRY(MOUSEEXCLUSIVE,       SETTINGS_CAT_OPTIONLIST, "MouseExclusive"  ),
        SAVEENTRY(ISFULLSCREENMODE,     SETTINGS_CAT_OPTIONLIST, "ISFULLSCREENMODE"),
        SAVEENTRY(WIDTH,                SETTINGS_CAT_OPTIONLIST, "WIDTH"           ),
        SAVEENTRY(HEIGHT,               SETTINGS_CAT_OPTIONLIST, "HEIGHT"          ),
        SAVEENTRY(BITPERPIXEL,          SETTINGS_CAT_OPTIONLIST, "BITPERPIXEL"     ),
        SAVEENTRY(DEVICECNT,            SETTINGS_CAT_OPTIONLIST, "DEVICECNT"       ),
        SAVEENTRY(MODECNT,              SETTINGS_CAT_OPTIONLIST, "MODECNT"         ),
        SAVEENTRY(SPRITEMODE,           SETTINGS_CAT_OPTIONLIST, "SPRITEMODE"      ),
        SAVEENTRY(TEXTUREMODE,          SETTINGS_CAT_OPTIONLIST, "TEXTUREMODE"     ),
        SAVEENTRY(BGMISPAUSED,          SETTINGS_CAT_COMMANDLIST, "/bgm"           ),
        SAVEENTRY(ISSOUNDON,            SETTINGS_CAT_COMMANDLIST, "/sound"         ),
        SAVEENTRY(NOTRADE,              SETTINGS_CAT_COMMANDLIST, "/notrade"       ),
        SAVEENTRY(NOSHIFT,              SETTINGS_CAT_COMMANDLIST, "/noshift"       ),
        SAVEENTRY(NOCTRL,               SETTINGS_CAT_COMMANDLIST, "/noctrl"        ),
        SAVEENTRY(SKILLFAIL,            SETTINGS_CAT_COMMANDLIST, "/skillfail"     ),
        SAVEENTRY(NOTALKMSG,            SETTINGS_CAT_COMMANDLIST, "/notalkmsg"     ),
        SAVEENTRY(NOTALKMSG2,           SETTINGS_CAT_COMMANDLIST, "/notalkmsg2"    ),
        SAVEENTRY(SHOWNAME,             SETTINGS_CAT_COMMANDLIST, "/showname"      ),
        SAVEENTRY(FOG,                  SETTINGS_CAT_COMMANDLIST, "/fog"           ),
        SAVEENTRY(AURA,                 SETTINGS_CAT_COMMANDLIST, "/aura"          ),
        SAVEENTRY(WINDOW,               SETTINGS_CAT_COMMANDLIST, "/window"        ),
        SAVEENTRY(MAKEMISSEFFECT,       SETTINGS_CAT_COMMANDLIST, "/miss"          ),
        SAVEENTRY(ISEFFECTON,           SETTINGS_CAT_COMMANDLIST, "/effect"        ),
        SAVEENTRY(SHOPPING,             SETTINGS_CAT_COMMANDLIST, "/shopping"      ),
        SAVEENTRY(STATEINFO,            SETTINGS_CAT_COMMANDLIST, "/stateinfo"     ),
        SAVEENTRY(LOGINOUT,             SETTINGS_CAT_COMMANDLIST, "/loginout"      ),
        SAVEENTRY(SNAP,                 SETTINGS_CAT_COMMANDLIST, "/snap"          ),
        SAVEENTRY(ISITEMSNAP,           SETTINGS_CAT_COMMANDLIST, "/itemsnap"      ),
        SAVEENTRY(SKILLSNAP,            SETTINGS_CAT_COMMANDLIST, "/skillsnap"     ),
        SAVEENTRY(ISFIXEDCAMERA,        SETTINGS_CAT_COMMANDLIST, "/camera"        ),
        SAVEENTRY(ONHOUSERAI,           SETTINGS_CAT_COMMANDLIST, "/hoai"          ),
        SAVEENTRY(ONMERUSERAI,          SETTINGS_CAT_COMMANDLIST, "/merai"         ),
        SAVEENTRY(ISLIGHTMAP,           SETTINGS_CAT_COMMANDLIST, "/lightmap"      ),
        SAVEENTRY(MONSTERHP,            SETTINGS_CAT_COMMANDLIST, "/monsterhp"     ),
        SAVEENTRY(Q1,                   SETTINGS_CAT_COMMANDLIST, "/q1"            ),
        SAVEENTRY(Q2,                   SETTINGS_CAT_COMMANDLIST, "/q2"            ),
#undef SAVEENTRY
    };

    if(this->m_nFlags)
    {
        bool bCanSave = true;

        if(this->m_nFlags&SF_RESET_UI)
        {
            this->ResetUI();
        }
        if(this->m_nFlags&SF_RESET_SKILLLEVEL)
        {
            this->ResetSkillLevel();
        }
        if(this->m_nFlags&SF_RESET_USERDATA)
        {
            this->ResetUserData();
        }
        if(this->m_nFlags&SF_RESET_SETTING)
        {
            this->ResetSettings();
            bCanSave = false;
        }

        if(!bCanSave)
        {
            return true;
        }
    }

    this->ResetInstall();

    lResult = RegCreateKeyEx(HKEY_GRAVITY, SETTINGS_RENEWPATH, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, NULL);

    if(lResult!=ERROR_SUCCESS)
    {
        SetLastError(lResult);
        CError::ErrorMessage(NULL, TEXT_ERROR_HKEY_CREATE);
        return false;
    }
    RegCloseKey(hKey);

    lResult = RegCreateKeyEx(HKEY_GRAVITY, SETTINGS_REGPATH, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, NULL);

    if(lResult!=ERROR_SUCCESS)
    {
        SetLastError(lResult);
        CError::ErrorMessage(NULL, TEXT_ERROR_HKEY_CREATE);
        return false;
    }

    if(!RegUtilSave(hKey, SaveInfo, __ARRAYSIZE(SaveInfo), &lResult))
    {
        SetLastError(lResult);
        CError::ErrorMessage(NULL, TEXT_ERROR_HKEY_WRITE);
    }

    RegFlushKey(hKey);
    RegCloseKey(hKey);

    // Lua
    bool bSuccess = false;
    FILE* hFile;

    this->m_Entries.BGMISPAUSED = !this->m_Entries.BGMISPAUSED;  // the meaning in LUA is reversed

    if(CreateDirectory(P_GetLuaPath(), NULL) || (GetLastError()==ERROR_ALREADY_EXISTS && (GetFileAttributes(P_GetLuaPath())&FILE_ATTRIBUTE_DIRECTORY /* make sure it's a directory */)))
    {
        unsigned long luIdx;
        CLuaIO L;

        // Define tables
        L.DefineTable(SETTINGS_CAT_COMMANDLIST);
        L.DefineTable(SETTINGS_CAT_OPTIONLIST);
        // L.Load("System\\LuaFiles514\\OptionInfo");  // official defaults
        L.Load(P_GetLuaFile(), CLuaIO::LUAFILETYPE::LFT_LUA);  // do not care about the outcome, as the file might not exist

        // Remove values we are going to update from state
        for(luIdx = 0; luIdx<__ARRAYSIZE(LuaSaveInfo); luIdx++)
        {
            L.DeleteTableValue(LuaSaveInfo[luIdx].lpszCategory, LuaSaveInfo[luIdx].lpszName);
        }

        // Write
        if((hFile = fopen(P_GetLuaFull(), "wb"))!=NULL)  // create new file, we have all of it in memory
        {
            // Credits
            fprintf(hFile,
                "--\r\n"
                "-- RagnarokOnline Settings File\r\n"
                "-- Generated by RagnarokOnline OpenSetup, Version "APP_VERSION" ( http://ai4rei.net/p/opensetup )\r\n"
                "--\r\n"
                "\r\n"
            );

            // 'Failback'
            fprintf(hFile,
                "-- Workaround for people with broken client folder\r\n"
                "-- Note: The client removes this when it saves settings.\r\n"
                "--\r\n"
                "if "SETTINGS_CAT_OPTIONLIST"==nil then\r\n"
                "\t"SETTINGS_CAT_OPTIONLIST" = {}\r\n"
                "end\r\n"
                "if "SETTINGS_CAT_COMMANDLIST"==nil then\r\n"
                "\t"SETTINGS_CAT_COMMANDLIST" = {}\r\n"
                "end\r\n"
                "-- End of Workaround\r\n"
                "\r\n"
            );

            // Body
            fprintf(hFile,
                "\r\n"
                "-- Settings\r\n"
                "--\r\n"
                "\r\n"
            );

            // Setup relevant settings first
            for(luIdx = 0; luIdx<__ARRAYSIZE(LuaSaveInfo); luIdx++)
            {
                if(fprintf(hFile, SETTINGS_SETMASK, LuaSaveInfo[luIdx].lpszCategory, LuaSaveInfo[luIdx].lpszName, LuaSaveInfo[luIdx].lpluValue[0])<1)
                {
                    CError::ErrorMessage(NULL, TEXT_ERROR_FILE_WRITE);
                    break;
                }
            }

            if(luIdx==__ARRAYSIZE(LuaSaveInfo))
            {
                bSuccess = true;
            }

            if(bSuccess)
            {
                // Client relevant settings
                fprintf(hFile,
                    "\r\n"
                    "-- Preserved client settings\r\n"
                    "--\r\n"
                    "\r\n"
                );

                L.ForEachTableKey(SETTINGS_CAT_OPTIONLIST, &CSettingsLua_P_ForEachKeySave, hFile);
                L.ForEachTableKey(SETTINGS_CAT_COMMANDLIST, &CSettingsLua_P_ForEachKeySave, hFile);
            }

            if(fflush(hFile)==EOF)
            {
                if(bSuccess)
                {
                    CError::ErrorMessage(NULL, TEXT_ERROR_FILE_WRITE);
                }

                bSuccess = false;
            }

            if(fclose(hFile)==EOF)
            {
                if(bSuccess)
                {
                    CError::ErrorMessage(NULL, TEXT_ERROR_FILE_WRITE);
                }

                bSuccess = false;
            }
        }
        else
        {
            DWORD dwAttr = GetFileAttributes(P_GetLuaFull());

            if(dwAttr!=INVALID_FILE_ATTRIBUTES && (dwAttr&FILE_ATTRIBUTE_READONLY))
            {
                CError::ErrorMessage(NULL, TEXT_ERROR_FILE_OPEN_READONLY);
            }
            else
            {
                CError::ErrorMessage(NULL, TEXT_ERROR_FILE_OPEN);
            }
        }
    }
    else
    {
        CError::ErrorMessage(NULL, TEXT_ERROR_DIRECTORY_CREATE);
    }

    this->m_Entries.BGMISPAUSED = !this->m_Entries.BGMISPAUSED;  // restore

    return bSuccess;
}

void __stdcall CSettingsLua::Load()
{
    unsigned long luDeviceNameLen, luGUIDDeviceLen, luGUIDDriverLen, luProviderNameLen;
    HKEY hKey;
    REGUTILLOADINFO LoadInfo[] =
    {
#define LOADENTRY(name,type,retlen) { #name, &this->m_Entries.##name, sizeof(this->m_Entries.##name), retlen, (type) }
        LOADENTRY(SOUNDMODE,        REG_DWORD,  NULL              ),
        LOADENTRY(SPEAKERTYPE,      REG_DWORD,  NULL              ),
        LOADENTRY(DIGITALRATETYPE,  REG_DWORD,  NULL              ),
        LOADENTRY(DIGITALBITSTYPE,  REG_DWORD,  NULL              ),
        LOADENTRY(GUIDDRIVER,       REG_BINARY, &luGUIDDriverLen  ),
        LOADENTRY(GUIDDEVICE,       REG_BINARY, &luGUIDDeviceLen  ),
        LOADENTRY(DEVICENAME,       REG_SZ,     &luDeviceNameLen  ),
        LOADENTRY(PROVIDERNAME,     REG_SZ,     &luProviderNameLen),
        LOADENTRY(SHOWTIPSATSTARTUP,REG_DWORD,  NULL              ),
#undef LOADENTRY
    };
    LUAOPTIONINFOINT LuaLoadInfo[] =
    {
#define LOADENTRY(name,cat,optname) { cat, optname, &this->m_Entries.##name }
        LOADENTRY(ISFULLSCREENMODE,     SETTINGS_CAT_OPTIONLIST, "ISFULLSCREENMODE"),
        LOADENTRY(WIDTH,                SETTINGS_CAT_OPTIONLIST, "WIDTH"           ),
        LOADENTRY(HEIGHT,               SETTINGS_CAT_OPTIONLIST, "HEIGHT"          ),
        LOADENTRY(BITPERPIXEL,          SETTINGS_CAT_OPTIONLIST, "BITPERPIXEL"     ),
        LOADENTRY(DEVICECNT,            SETTINGS_CAT_OPTIONLIST, "DEVICECNT"       ),
        LOADENTRY(MODECNT,              SETTINGS_CAT_OPTIONLIST, "MODECNT"         ),
        LOADENTRY(ISLIGHTMAP,           SETTINGS_CAT_COMMANDLIST, "/lightmap"      ),
        LOADENTRY(SPRITEMODE,           SETTINGS_CAT_OPTIONLIST, "SPRITEMODE"      ),
        LOADENTRY(TEXTUREMODE,          SETTINGS_CAT_OPTIONLIST, "TEXTUREMODE"     ),
        LOADENTRY(FOG,                  SETTINGS_CAT_COMMANDLIST, "/fog"           ),
        LOADENTRY(TRILINEARFILTER,      SETTINGS_CAT_OPTIONLIST, "Trilinear"       ),
        LOADENTRY(STREAMVOLUME,         SETTINGS_CAT_OPTIONLIST, "Bgm_Volume"      ),
        LOADENTRY(SOUNDVOLUME,          SETTINGS_CAT_OPTIONLIST, "Effect_Volume"   ),
        LOADENTRY(MOUSEEXCLUSIVE,       SETTINGS_CAT_OPTIONLIST, "MouseExclusive"  ),
        LOADENTRY(BGMISPAUSED,          SETTINGS_CAT_COMMANDLIST, "/bgm"           ),
        LOADENTRY(ISSOUNDON,            SETTINGS_CAT_COMMANDLIST, "/sound"         ),
        LOADENTRY(NOTRADE,              SETTINGS_CAT_COMMANDLIST, "/notrade"       ),
        LOADENTRY(NOSHIFT,              SETTINGS_CAT_COMMANDLIST, "/noshift"       ),
        LOADENTRY(NOCTRL,               SETTINGS_CAT_COMMANDLIST, "/noctrl"        ),
        LOADENTRY(SKILLFAIL,            SETTINGS_CAT_COMMANDLIST, "/skillfail"     ),
        LOADENTRY(NOTALKMSG,            SETTINGS_CAT_COMMANDLIST, "/notalkmsg"     ),
        LOADENTRY(NOTALKMSG2,           SETTINGS_CAT_COMMANDLIST, "/notalkmsg2"    ),
        LOADENTRY(SHOWNAME,             SETTINGS_CAT_COMMANDLIST, "/showname"      ),
        LOADENTRY(AURA,                 SETTINGS_CAT_COMMANDLIST, "/aura"          ),
        LOADENTRY(WINDOW,               SETTINGS_CAT_COMMANDLIST, "/window"        ),
        LOADENTRY(MAKEMISSEFFECT,       SETTINGS_CAT_COMMANDLIST, "/miss"          ),
        LOADENTRY(ISEFFECTON,           SETTINGS_CAT_COMMANDLIST, "/effect"        ),
        LOADENTRY(SHOPPING,             SETTINGS_CAT_COMMANDLIST, "/shopping"      ),
        LOADENTRY(STATEINFO,            SETTINGS_CAT_COMMANDLIST, "/stateinfo"     ),
        LOADENTRY(LOGINOUT,             SETTINGS_CAT_COMMANDLIST, "/loginout"      ),
        LOADENTRY(SNAP,                 SETTINGS_CAT_COMMANDLIST, "/snap"          ),
        LOADENTRY(ISITEMSNAP,           SETTINGS_CAT_COMMANDLIST, "/itemsnap"      ),
        LOADENTRY(SKILLSNAP,            SETTINGS_CAT_COMMANDLIST, "/skillsnap"     ),
        LOADENTRY(ISFIXEDCAMERA,        SETTINGS_CAT_COMMANDLIST, "/camera"        ),
        LOADENTRY(ONHOUSERAI,           SETTINGS_CAT_COMMANDLIST, "/hoai"          ),
        LOADENTRY(ONMERUSERAI,          SETTINGS_CAT_COMMANDLIST, "/merai"         ),
        LOADENTRY(MONSTERHP,            SETTINGS_CAT_COMMANDLIST, "/monsterhp"     ),
        LOADENTRY(Q1,                   SETTINGS_CAT_COMMANDLIST, "/q1"            ),
        LOADENTRY(Q2,                   SETTINGS_CAT_COMMANDLIST, "/q2"            ),
#undef LOADENTRY
    };

    this->Reset();

    if(RegOpenKeyEx(HKEY_GRAVITY, SETTINGS_REGPATH, 0, KEY_READ, &hKey)==ERROR_SUCCESS)
    {
        RegUtilLoad(hKey, LoadInfo, __ARRAYSIZE(LoadInfo), NULL);
        RegCloseKey(hKey);
    }

    // Lua
    CLuaIO L;

    // Define tables
    L.DefineTable(SETTINGS_CAT_COMMANDLIST);
    L.DefineTable(SETTINGS_CAT_OPTIONLIST);
    // L.Load("System\\LuaFiles514\\OptionInfo");  // official defaults

    if(L.Load(P_GetLuaFile(), CLuaIO::LUAFILETYPE::LFT_LUA))
    {
        unsigned long i;

        for(i = 0; i<__ARRAYSIZE(LuaLoadInfo); i++)
        {
            L.GetTableInteger(LuaLoadInfo[i].lpszCategory, LuaLoadInfo[i].lpszName, LuaLoadInfo[i].lpluValue);
        }
    }

    this->m_Entries.BGMISPAUSED = !this->m_Entries.BGMISPAUSED;  // the meaning in LUA is reversed
}

void __stdcall CSettingsLua::Reset()
{
    GUID Guid;

    ZeroMemory(&Guid, sizeof(Guid));

    this->Set(SE_ISFULLSCREENMODE, 0UL          );
    this->Set(SE_WIDTH,            640UL        );
    this->Set(SE_HEIGHT,           480UL        );
    this->Set(SE_BITPERPIXEL,      16UL         );
    this->Set(SE_DEVICECNT,        0UL          );
    this->Set(SE_MODECNT,          0UL          );
    this->Set(SE_ISVOODOO,         0UL          );
    this->Set(SE_ISLIGHTMAP,       1UL          );
    this->Set(SE_SPRITEMODE,       2UL          );
    this->Set(SE_TEXTUREMODE,      2UL          );
    this->Set(SE_NUMSAMPLETYPE,    2UL          );
    this->Set(SE_FOG,              1UL          );
    this->Set(SE_SOUNDMODE,        1UL          );
    this->Set(SE_SPEAKERTYPE,      0UL          );
    this->Set(SE_DIGITALRATETYPE,  0UL          );
    this->Set(SE_DIGITALBITSTYPE,  0UL          );
    this->Set(SE_GUIDDRIVER,       &Guid        );
    this->Set(SE_GUIDDEVICE,       &Guid        );
    this->Set(SE_DEVICENAME,       ""           );
    this->Set(SE_PROVIDERNAME,     "No Provider");  // NOTE: Official setup saves "8—A", even though it intends this value.
    //
    this->Set(SE_SHOWTIPSATSTARTUP,1UL          );
    this->Set(SE_TRILINEARFILTER,  0UL          );
    //
    this->Set(SE_STREAMVOLUME,     100UL        );
    this->Set(SE_SOUNDVOLUME,      100UL        );
    this->Set(SE_MOUSEEXCLUSIVE,   1UL          );
    this->Set(SE_BGMISPAUSED,      1UL          );  // HACK: account for /bgm = 1 (loader reverses it again)
    this->Set(SE_ISSOUNDON,        1UL          );
    this->Set(SE_NOTRADE,          0UL          );
    this->Set(SE_NOSHIFT,          0UL          );
    this->Set(SE_NOCTRL,           1UL          );
    this->Set(SE_SKILLFAIL,        1UL          );
    this->Set(SE_NOTALKMSG,        0UL          );
    this->Set(SE_NOTALKMSG2,       0UL          );
    this->Set(SE_SHOWNAME,         1UL          );
    this->Set(SE_AURA,             1UL          );
    this->Set(SE_WINDOW,           0UL          );
    this->Set(SE_MAKEMISSEFFECT,   1UL          );
    this->Set(SE_ISEFFECTON,       1UL          );
    this->Set(SE_SHOPPING,         1UL          );
    this->Set(SE_STATEINFO,        1UL          );
    this->Set(SE_LOGINOUT,         1UL          );
    this->Set(SE_SNAP,             0UL          );
    this->Set(SE_ISITEMSNAP,       0UL          );
    this->Set(SE_SKILLSNAP,        1UL          );
    this->Set(SE_ISFIXEDCAMERA,    0UL          );
    this->Set(SE_ONHOUSERAI,       0UL          );
    this->Set(SE_ONMERUSERAI,      0UL          );
    this->Set(SE_MONSTERHP,        1UL          );
    this->Set(SE_Q1,               0UL          );
    this->Set(SE_Q2,               0UL          );
    //
    this->Set(SF__ALL_FLAGS, false);
}

bool __stdcall CSettingsLua::IsAvail(SETTINGENTRY nEntry)
{
    switch(nEntry)
    {
        case SE_ISFULLSCREENMODE:
        case SE_WIDTH:
        case SE_HEIGHT:
        case SE_BITPERPIXEL:
        case SE_DEVICECNT:
        case SE_MODECNT:
        //case SE_ISVOODOO:
        case SE_ISLIGHTMAP:
        case SE_SPRITEMODE:
        case SE_TEXTUREMODE:
        //case SE_NUMSAMPLETYPE:
        case SE_FOG:
        case SE_SOUNDMODE:
        //case SE_SPEAKERTYPE:
        //case SE_DIGITALRATETYPE:
        //case SE_DIGITALBITSTYPE:
        case SE_GUIDDRIVER:
        case SE_GUIDDEVICE:
        case SE_DEVICENAME:
        case SE_PROVIDERNAME:
        //
        case SE_SHOWTIPSATSTARTUP:
        case SE_TRILINEARFILTER:
        //
        case SE_STREAMVOLUME:
        case SE_SOUNDVOLUME:
        case SE_MOUSEEXCLUSIVE:
        case SE_BGMISPAUSED:
        case SE_ISSOUNDON:
        case SE_NOTRADE:
        case SE_NOSHIFT:
        case SE_NOCTRL:
        case SE_SKILLFAIL:
        case SE_NOTALKMSG:
        case SE_NOTALKMSG2:
        case SE_SHOWNAME:
        case SE_AURA:
        case SE_WINDOW:
        case SE_MAKEMISSEFFECT:
        case SE_ISEFFECTON:
        case SE_SHOPPING:
        case SE_STATEINFO:
        case SE_LOGINOUT:
        case SE_SNAP:
        case SE_ISITEMSNAP:
        case SE_SKILLSNAP:
        case SE_ISFIXEDCAMERA:
        case SE_ONHOUSERAI:
        case SE_ONMERUSERAI:
        case SE_MONSTERHP:
        case SE_Q1:
        case SE_Q2:
            return true;
    }

    return false;
}

bool __stdcall CSettingsLua::IsAdminRequired()
{
    OSVERSIONINFO Osvi = { sizeof(Osvi) };

    GetVersionEx(&Osvi);

    if(Osvi.dwPlatformId==VER_PLATFORM_WIN32_NT)
    {
        HKEY hKey;

        if(RegCreateKeyEx(HKEY_GRAVITY, SETTINGS_REGPATH, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, NULL)!=ERROR_SUCCESS)
        {
            return true;
        }

        RegCloseKey(hKey);

        // if the lua folder does not exist yet, we need write rights in the current folder
        const char* lpszPath = (GetFileAttributes(P_GetLuaPath())==INVALID_FILE_ATTRIBUTES) ? "." : P_GetLuaPath();

        // attempt to open the folder
        HANDLE hFolder = CreateFile(
            lpszPath,                                                                   // path
            FILE_ADD_FILE|FILE_ADD_SUBDIRECTORY|FILE_WRITE_DATA|FILE_WRITE_ATTRIBUTES,  // any kind of write operation
            FILE_SHARE_READ|FILE_SHARE_WRITE,                                           // any kind of sharing
            NULL,                                                                       // not inhertable
            OPEN_EXISTING,                                                              // must exist, otherwise unwritable
            FILE_FLAG_BACKUP_SEMANTICS,                                                 // required for folder handles
            NULL                                                                        // no template for obvious reasons
        );

        if(hFolder==INVALID_HANDLE_VALUE)
        {// no write rights, virtualization would jump in
            return true;
        }

        CloseHandle(hFolder);
    }

    return false;
}

bool __stdcall CSettingsLua::IsSane()
{
    bool bSuccess = true;
    unsigned long luIdx;

    g_Log.LogInfo("LUA settings engine sanity check started.");
    g_Log.IncrementLevel();

    {// what has to be there
        const char* lppszFiles[] =
        {
            "System\\LuaFiles514\\OptionInfo.lub",
            "System\\OptionInfo.lub",
        };

        for(luIdx = 0; luIdx<__ARRAYSIZE(lppszFiles); luIdx++)
        {
            g_Log.LogInfo("Checking for '%s'...", lppszFiles[luIdx]);

            if(GetFileAttributes(lppszFiles[luIdx])==INVALID_FILE_ATTRIBUTES)
            {
                ;
            }
            else
            {
                HANDLE hFile = CreateFile(lppszFiles[luIdx], GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);

                if(hFile==INVALID_HANDLE_VALUE)
                {
                    g_Log.LogInfo("Default settings file '%s' found, but it cannot be read (code=%u).", lppszFiles[luIdx], GetLastError());
                }
                else
                {
                    CloseHandle(hFile);

                    g_Log.LogInfo("Default settings file '%s' found. Continuing...", lppszFiles[luIdx]);
                    break;
                }
            }
        }

        if(luIdx==__ARRAYSIZE(lppszFiles))
        {
            g_Log.LogError("No usable default settings file was found, your client won't be able to read settings.");

            bSuccess = false;
        }
    }

    if(GetFileAttributes(P_GetLuaPath())!=INVALID_FILE_ATTRIBUTES)
    {// what should be accessible if it is there
        const char* lppszFiles[] =
        {
            P_GetLuaFull(),
        };

        for(luIdx = 0; luIdx<__ARRAYSIZE(lppszFiles); luIdx++)
        {
            HANDLE hFile;

            g_Log.LogInfo("Checking for '%s'...", lppszFiles[luIdx]);

            hFile = CreateFile(lppszFiles[luIdx], GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL|FILE_FLAG_DELETE_ON_CLOSE, NULL);

            if(hFile==INVALID_HANDLE_VALUE)
            {
                switch(GetLastError())
                {
                    case ERROR_FILE_EXISTS:
                        hFile = CreateFile(lppszFiles[luIdx], GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

                        if(hFile==INVALID_HANDLE_VALUE)
                        {
                            switch(GetLastError())
                            {
                                case ERROR_SHARING_VIOLATION:
                                    g_Log.LogError("Settings file '%s' found, but some application is blocking access to it.", lppszFiles[luIdx]);
                                    break;
                                case ERROR_ACCESS_DENIED:
                                {
                                    DWORD dwAttributes = GetFileAttributes(lppszFiles[luIdx]);

                                    if(dwAttributes==INVALID_FILE_ATTRIBUTES)
                                    {
                                        g_Log.LogError("Settings file '%s' found, but it cannot be accessed (code=%u).", lppszFiles[luIdx], GetLastError());
                                    }
                                    else if(dwAttributes&(FILE_ATTRIBUTE_HIDDEN|FILE_ATTRIBUTE_READONLY|FILE_ATTRIBUTE_SYSTEM))
                                    {
                                        g_Log.LogError("Settings file '%s' found, but it has protective attributes set (attr=0x%x).", lppszFiles[luIdx], dwAttributes);
                                    }
                                    else
                                    {
                                        g_Log.LogError("Settings file '%s' found, but access to it was explicitely denied (attr=0x%x).", lppszFiles[luIdx], dwAttributes);
                                    }

                                    break;
                                }
                                default:
                                    g_Log.LogError("Settings file '%s' found, but it cannot be read or written (code=%u).", lppszFiles[luIdx], GetLastError());
                                    break;
                            }

                            bSuccess = false;
                        }
                        else
                        {
                            CloseHandle(hFile);

                            g_Log.LogInfo("Settings file '%s' found, can be both read and written.", lppszFiles[luIdx]);
                        }

                        break;
                    case ERROR_ACCESS_DENIED:
                    {
                        DWORD dwAttributes = GetFileAttributes(lppszFiles[luIdx]);

                        if(dwAttributes==INVALID_FILE_ATTRIBUTES)
                        {
                            g_Log.LogError("Settings file '%s' not found, and creation was explicitely denied.", lppszFiles[luIdx]);
                        }
                        else if(dwAttributes&FILE_ATTRIBUTE_DIRECTORY)
                        {
                            g_Log.LogError("Settings file '%s' not found, but a directory with same name already exists (attr=0x%x).", lppszFiles[luIdx], dwAttributes);
                        }
                        else
                        {
                            g_Log.LogError("Settings file '%s' not found, but some other file system object with same name already exists (attr=0x%x).", lppszFiles[luIdx], dwAttributes);
                        }

                        bSuccess = false;
                        break;
                    }
                    default:
                        g_Log.LogError("Settings file '%s' not found, and it cannot be created either (code=%u).", lppszFiles[luIdx], GetLastError());

                        bSuccess = false;
                        break;
                }
            }
            else
            {
                CloseHandle(hFile);

                g_Log.LogInfo("Settings file '%s' not found, but it can be created.", lppszFiles[luIdx]);
            }
        }
    }
    else
    {
        g_Log.LogInfo("Settings directory does not exist, skipping settings file tests.");
    }

    {// what has not to be there
        char szBadLub[MAX_PATH];

        snprintf(szBadLub, __ARRAYSIZE(szBadLub), "%s.lub", P_GetLuaFile());

        const char* lppszFiles[] =
        {
            szBadLub,
        };

        for(luIdx = 0; luIdx<__ARRAYSIZE(lppszFiles); luIdx++)
        {
            g_Log.LogInfo("Checking for '%s'...", lppszFiles[luIdx]);

            if(GetFileAttributes(lppszFiles[luIdx])==INVALID_FILE_ATTRIBUTES)
            {
                ;
            }
            else
            {
                g_Log.LogWarning("File '%s' found.", lppszFiles[luIdx]);
                break;
            }
        }

        if(luIdx!=__ARRAYSIZE(lppszFiles))
        {
            g_Log.LogError("Remove *.lub files from your savedata or your client won't be able to read settings.");

            bSuccess = false;
        }
    }

    g_Log.DecrementLevel();
    g_Log.LogInfo("LUA settings engine sanity check stopped.");

    return bSuccess;
}

SETTINGENGINEID __stdcall CSettingsLua::GetEngineID()
{
    return SENGINE_LUA;
}


void __stdcall CSettingsLua::ResetSettings()
{
    /* this:: */DropFolderList(l_lppszResetFileList, __ARRAYSIZE(l_lppszResetFileList));
    RegUtilDrop(HKEY_GRAVITY, SETTINGS_REGPATH);
    RegUtilDrop(HKEY_GRAVITY, SETTINGS_RENEWPATH);
}
