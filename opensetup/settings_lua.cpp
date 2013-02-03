// -----------------------------------------------------------------
// RagnarokOnline OpenSetup
// (c) 2010-2013 Ai4rei/AN
// See doc/license.txt for details.
//
// -----------------------------------------------------------------

#include <stdio.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <regutil.h>

#include "opensetup.h"
#include "luaio.h"
#include "settings.h"
#include "settings_lua.h"

//#define HKLM_TO_HKCU

#define SETTINGS_REGPATH "Software\\Gravity Soft\\Ragnarok"
#define SETTINGS_RENEWPATH "Software\\Gravity Soft\\RenewSetup"
#define SETTINGS_LUAPATH "savedata"
#define SETTINGS_LUAFILE SETTINGS_LUAPATH"\\OptionInfo"
#define SETTINGS_LUAFULL SETTINGS_LUAFILE".lua"
#define SETTINGS_SETMASK "%s[\"%s\"] = %d\r\n"
#define SETTINGS_CAT_OPTIONLIST "OptionInfoList"
#define SETTINGS_CAT_COMMANDLIST "CmdOnOffList"

struct LuaOptionInfoInt
{
    const char* lpszCategory;
    const char* lpszName;
    unsigned long* lpluValue;  // there is no non-number saving in LUA right now
};

#ifndef HKLM_TO_HKCU
    #define HKEY_GRAVITY HKEY_LOCAL_MACHINE
#else
    #define HKEY_GRAVITY HKEY_CURRENT_USER
#endif

void __stdcall CSettingsLua::Save(void)
{
    HKEY hKey;
    struct RegUtilSaveInfo SaveInfo[] =
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
    struct LuaOptionInfoInt LuaSaveInfo[] =
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
        SAVEENTRY(ISFIXEDCAMERA,        SETTINGS_CAT_COMMANDLIST, "/camera"        ),
        SAVEENTRY(ONHOUSERAI,           SETTINGS_CAT_COMMANDLIST, "/hoai"          ),
        SAVEENTRY(ONMERUSERAI,          SETTINGS_CAT_COMMANDLIST, "/merai"         ),
        SAVEENTRY(ISLIGHTMAP,           SETTINGS_CAT_COMMANDLIST, "/lightmap"      ),
        // /btg
        //SAVEENTRY(ISVOODOO,           , "" ),
        //SAVEENTRY(NUMSAMPLETYPE,      , "" ),
        //SAVEENTRY(SOUNDMODE,          , "" ),  // still uses REG
        //SAVEENTRY(SPEAKERTYPE,        , "" ),  // still uses REG
        //SAVEENTRY(DIGITALRATETYPE,    , "" ),  // still uses REG
        //SAVEENTRY(DIGITALBITSTYPE,    , "" ),  // still uses REG
        //SAVEENTRY(GUIDDRIVER,         , "" ),  // still uses REG
        //SAVEENTRY(GUIDDEVICE,         , "" ),  // still uses REG
        //SAVEENTRY(DEVICENAME,         , "" ),  // still uses REG
        //SAVEENTRY(PROVIDERNAME,       , "" ),  // still uses REG
        //SAVEENTRY(SHOWTIPSATSTARTUP,  , "" ),
#undef SAVEENTRY
    };

    if(RegCreateKeyEx(HKEY_GRAVITY, SETTINGS_RENEWPATH, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, NULL)==ERROR_SUCCESS)
    {
        RegCloseKey(hKey);
    }

    if(RegCreateKeyEx(HKEY_GRAVITY, SETTINGS_REGPATH, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, NULL)!=ERROR_SUCCESS)
    {
        MessageBox(NULL, "Failed to save settings. Not enough privileges?", "Error - OpenSetup", MB_OK|MB_ICONSTOP);
        return;
    }

    if(!RegUtilSave(hKey, SaveInfo, __ARRAYSIZE(SaveInfo)))
    {
        MessageBox(NULL, "Failed to save settings. Not enough privileges?", "Error - OpenSetup", MB_OK|MB_ICONSTOP);
    }

    RegFlushKey(hKey);
    RegCloseKey(hKey);

    // Lua
    // Gravity is lazy as we are in this part. They just append all
    // values to the possibly existing ones, so that options saved
    // by the client do not get lost, yet the ones set by setup get
    // updated. An ever-growing pile of sh-, i mean settings...
    FILE* hFile;

    this->m_Entries.BGMISPAUSED = !this->m_Entries.BGMISPAUSED;  // the meaning in LUA is reversed

    if((CreateDirectory(SETTINGS_LUAPATH, NULL) || GetLastError()==ERROR_ALREADY_EXISTS) && (hFile = fopen(SETTINGS_LUAFULL, "ab"))!=NULL)
    {
        unsigned long i;

        for(i = 0; i<__ARRAYSIZE(LuaSaveInfo); i++)
        {
            if(fprintf(hFile, SETTINGS_SETMASK, LuaSaveInfo[i].lpszCategory, LuaSaveInfo[i].lpszName, LuaSaveInfo[i].lpluValue[0])<1)
            {
                MessageBox(NULL, "Failed to save settings. Disk full?", "Error - OpenSetup", MB_OK|MB_ICONSTOP);
                break;
            }
        }

        fclose(hFile);
    }
    else
    {
        MessageBox(NULL, "Failed to save settings. Not enough privileges?", "Error - OpenSetup", MB_OK|MB_ICONSTOP);
    }

    this->m_Entries.BGMISPAUSED = !this->m_Entries.BGMISPAUSED;  // restore
}

void __stdcall CSettingsLua::Load(void)
{
    unsigned long luDeviceNameLen, luGUIDDeviceLen, luGUIDDriverLen, luProviderNameLen;
    HKEY hKey;
    struct RegUtilLoadInfo LoadInfo[] =
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
    struct LuaOptionInfoInt LuaLoadInfo[] =
    {
#define LOADENTRY(name,cat,optname) { cat, optname, &this->m_Entries.##name }
        LOADENTRY(ISFULLSCREENMODE,     SETTINGS_CAT_OPTIONLIST, "ISFULLSCREENMODE"),
        LOADENTRY(WIDTH,                SETTINGS_CAT_OPTIONLIST, "WIDTH"           ),
        LOADENTRY(HEIGHT,               SETTINGS_CAT_OPTIONLIST, "HEIGHT"          ),
        LOADENTRY(BITPERPIXEL,          SETTINGS_CAT_OPTIONLIST, "BITPERPIXEL"     ),
        LOADENTRY(DEVICECNT,            SETTINGS_CAT_OPTIONLIST, "DEVICECNT"       ),
        LOADENTRY(MODECNT,              SETTINGS_CAT_OPTIONLIST, "MODECNT"         ),
        //LOADENTRY(ISVOODOO,           , "" ),
        LOADENTRY(ISLIGHTMAP,           SETTINGS_CAT_COMMANDLIST, "/lightmap"      ),
        LOADENTRY(SPRITEMODE,           SETTINGS_CAT_OPTIONLIST, "SPRITEMODE"      ),
        LOADENTRY(TEXTUREMODE,          SETTINGS_CAT_OPTIONLIST, "TEXTUREMODE"     ),
        //LOADENTRY(NUMSAMPLETYPE,      , "" ),
        LOADENTRY(FOG,                  SETTINGS_CAT_COMMANDLIST, "/fog"           ),
        //LOADENTRY(SOUNDMODE,          , "" ),
        //LOADENTRY(SPEAKERTYPE,        , "" ),
        //LOADENTRY(DIGITALRATETYPE,    , "" ),
        //LOADENTRY(DIGITALBITSTYPE,    , "" ),
        //LOADENTRY(GUIDDRIVER,         , "" ),
        //LOADENTRY(GUIDDEVICE,         , "" ),
        //LOADENTRY(DEVICENAME,         , "" ),
        //LOADENTRY(PROVIDERNAME,       , "" ),
        //LOADENTRY(SHOWTIPSATSTARTUP,  , "" ),
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
        LOADENTRY(ISFIXEDCAMERA,        SETTINGS_CAT_COMMANDLIST, "/camera"        ),
        LOADENTRY(ONHOUSERAI,           SETTINGS_CAT_COMMANDLIST, "/hoai"          ),
        LOADENTRY(ONMERUSERAI,          SETTINGS_CAT_COMMANDLIST, "/merai"         ),
#undef LOADENTRY
    };

    this->Reset();

    if(RegOpenKeyEx(HKEY_GRAVITY, SETTINGS_REGPATH, 0, KEY_READ, &hKey)==ERROR_SUCCESS)
    {
        RegUtilLoad(hKey, LoadInfo, __ARRAYSIZE(LoadInfo));
        RegCloseKey(hKey);
    }

    // Lua
    CLuaIO L;
    // TODO: Does official setup do this, too? (or do we need to include something else as well?)
    L.DefineTable(SETTINGS_CAT_OPTIONLIST);
    L.DefineTable(SETTINGS_CAT_COMMANDLIST);

    if(L.Load(SETTINGS_LUAFILE))
    {
        unsigned long i;

        for(i = 0; i<__ARRAYSIZE(LuaLoadInfo); i++)
        {
            L.GetTableInteger(LuaLoadInfo[i].lpszCategory, LuaLoadInfo[i].lpszName, LuaLoadInfo[i].lpluValue);
        }
    }

    this->m_Entries.BGMISPAUSED = !this->m_Entries.BGMISPAUSED;  // the meaning in LUA is reversed
}

void __stdcall CSettingsLua::Reset(void)
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
    this->Set(SE_PROVIDERNAME,     "8—A");
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
    this->Set(SE_ISFIXEDCAMERA,    0UL          );
    this->Set(SE_ONHOUSERAI,       0UL          );
    this->Set(SE_ONMERUSERAI,      0UL          );
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
        case SE_ISFIXEDCAMERA:
        case SE_ONHOUSERAI:
        case SE_ONMERUSERAI:
            return true;
    }

    return false;
}

bool __stdcall CSettingsLua::IsAdminRequired(void)
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
        const char* lpszPath = (GetFileAttributes(SETTINGS_LUAPATH)==INVALID_FILE_ATTRIBUTES) ? "." : SETTINGS_LUAPATH;

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

SETTINGENGINEID __stdcall CSettingsLua::GetEngineID(void)
{
    return SENGINE_LUA;
}
