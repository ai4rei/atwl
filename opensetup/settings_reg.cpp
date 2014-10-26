// -----------------------------------------------------------------
// RagnarokOnline OpenSetup
// (c) 2010-2014 Ai4rei/AN
// See doc/license.txt for details.
//
// -----------------------------------------------------------------

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <regutil.h>

#include "opensetup.h"
#include "error.h"
#include "resource.h"
#include "settings.h"
#include "settings_reg.h"

#define SETTINGS_REGPATH "Software\\Gravity Soft\\Ragnarok"
#define SETTINGS_REGPATH_OPTION SETTINGS_REGPATH"\\Option"

bool __stdcall CSettingsReg::Save()
{
    HKEY hKey;
    LONG lResult;
    REGUTILSAVEINFO SaveInfo[] =
    {
#define SAVEENTRY(name,type) { #name, &this->m_Entries.##name, sizeof(this->m_Entries.##name), (type) }
        SAVEENTRY(ISFULLSCREENMODE, REG_DWORD ),
        SAVEENTRY(WIDTH,            REG_DWORD ),
        SAVEENTRY(HEIGHT,           REG_DWORD ),
        SAVEENTRY(BITPERPIXEL,      REG_DWORD ),
        SAVEENTRY(DEVICECNT,        REG_DWORD ),
        SAVEENTRY(MODECNT,          REG_DWORD ),
        SAVEENTRY(ISVOODOO,         REG_DWORD ),
        SAVEENTRY(ISLIGHTMAP,       REG_DWORD ),
        SAVEENTRY(SPRITEMODE,       REG_DWORD ),
        SAVEENTRY(TEXTUREMODE,      REG_DWORD ),
        SAVEENTRY(NUMSAMPLETYPE,    REG_DWORD ),
        SAVEENTRY(FOG,              REG_DWORD ),
        SAVEENTRY(SOUNDMODE,        REG_DWORD ),
        SAVEENTRY(SPEAKERTYPE,      REG_DWORD ),
        SAVEENTRY(DIGITALRATETYPE,  REG_DWORD ),
        SAVEENTRY(DIGITALBITSTYPE,  REG_DWORD ),
        SAVEENTRY(GUIDDRIVER,       REG_BINARY),
        SAVEENTRY(GUIDDEVICE,       REG_BINARY),
        SAVEENTRY(DEVICENAME,       REG_SZ    ),
        SAVEENTRY(PROVIDERNAME,     REG_SZ    ),
        SAVEENTRY(SHOWTIPSATSTARTUP,REG_DWORD ),
        SAVEENTRY(TRILINEARFILTER,  REG_DWORD ),
#undef SAVEENTRY
    };
    REGUTILSAVEINFO SaveOptionInfo[] =
    {
#define SAVEENTRY(name,type,optname) { optname, &this->m_Entries.##name, sizeof(this->m_Entries.##name), (type) }
        SAVEENTRY(STREAMVOLUME,     REG_DWORD, "streamVolume"           ),
        SAVEENTRY(SOUNDVOLUME,      REG_DWORD, "soundVolume"            ),
        SAVEENTRY(BGMISPAUSED,      REG_DWORD, "bgmIsPaused"            ),
        SAVEENTRY(ISSOUNDON,        REG_DWORD, "isSoundOn"              ),
        SAVEENTRY(NOCTRL,           REG_DWORD, "m_isNoCtrl"             ),
        SAVEENTRY(MAKEMISSEFFECT,   REG_DWORD, "m_bMakeMissEffect"      ),
        SAVEENTRY(ISEFFECTON,       REG_DWORD, "isEffectOn"             ),
        SAVEENTRY(SNAP,             REG_DWORD, "m_monsterSnapOn_NoSkill"),
        SAVEENTRY(ISITEMSNAP,       REG_DWORD, "m_isItemSnap"           ),
        SAVEENTRY(SKILLSNAP,        REG_DWORD, "m_monsterSnapOn_Skill"  ),
        SAVEENTRY(ISFIXEDCAMERA,    REG_DWORD, "g_isFixedCamera"        ),
        SAVEENTRY(ONHOUSERAI,       REG_DWORD, "onHoUserAI"             ),
        SAVEENTRY(ONMERUSERAI,      REG_DWORD, "onMerUserAI"            ),
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

    lResult = RegCreateKeyEx(HKEY_GRAVITY, SETTINGS_REGPATH_OPTION, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL);

    if(lResult!=ERROR_SUCCESS)
    {
        SetLastError(lResult);
        CError::ErrorMessage(NULL, TEXT_ERROR_HKEY_OPT_CREATE);
        return false;
    }

    if(!RegUtilSave(hKey, SaveOptionInfo, __ARRAYSIZE(SaveOptionInfo), &lResult))
    {
        SetLastError(lResult);
        CError::ErrorMessage(NULL, TEXT_ERROR_HKEY_OPT_WRITE);
    }

    RegFlushKey(hKey);
    RegCloseKey(hKey);
    return true;
}

void __stdcall CSettingsReg::Load()
{
    unsigned long luDeviceNameLen, luGUIDDeviceLen, luGUIDDriverLen, luProviderNameLen;
    HKEY hKey;
    REGUTILLOADINFO LoadInfo[] =
    {
#define LOADENTRY(name,type,retlen) { #name, &this->m_Entries.##name, sizeof(this->m_Entries.##name), retlen, (type) }
        LOADENTRY(ISFULLSCREENMODE, REG_DWORD,  NULL              ),
        LOADENTRY(WIDTH,            REG_DWORD,  NULL              ),
        LOADENTRY(HEIGHT,           REG_DWORD,  NULL              ),
        LOADENTRY(BITPERPIXEL,      REG_DWORD,  NULL              ),
        LOADENTRY(DEVICECNT,        REG_DWORD,  NULL              ),
        LOADENTRY(MODECNT,          REG_DWORD,  NULL              ),
        LOADENTRY(ISVOODOO,         REG_DWORD,  NULL              ),
        LOADENTRY(ISLIGHTMAP,       REG_DWORD,  NULL              ),
        LOADENTRY(SPRITEMODE,       REG_DWORD,  NULL              ),
        LOADENTRY(TEXTUREMODE,      REG_DWORD,  NULL              ),
        LOADENTRY(NUMSAMPLETYPE,    REG_DWORD,  NULL              ),
        LOADENTRY(FOG,              REG_DWORD,  NULL              ),
        LOADENTRY(SOUNDMODE,        REG_DWORD,  NULL              ),
        LOADENTRY(SPEAKERTYPE,      REG_DWORD,  NULL              ),
        LOADENTRY(DIGITALRATETYPE,  REG_DWORD,  NULL              ),
        LOADENTRY(DIGITALBITSTYPE,  REG_DWORD,  NULL              ),
        LOADENTRY(GUIDDRIVER,       REG_BINARY, &luGUIDDriverLen  ),
        LOADENTRY(GUIDDEVICE,       REG_BINARY, &luGUIDDeviceLen  ),
        LOADENTRY(DEVICENAME,       REG_SZ,     &luDeviceNameLen  ),
        LOADENTRY(PROVIDERNAME,     REG_SZ,     &luProviderNameLen),
        LOADENTRY(SHOWTIPSATSTARTUP,REG_DWORD,  NULL              ),
        LOADENTRY(TRILINEARFILTER,  REG_DWORD,  NULL              ),
#undef LOADENTRY
    };
    REGUTILLOADINFO LoadOptionInfo[] =
    {
#define LOADENTRY(name,type,optname) { optname, &this->m_Entries.##name, sizeof(this->m_Entries.##name), NULL, (type) }
        LOADENTRY(STREAMVOLUME,     REG_DWORD, "streamVolume"           ),
        LOADENTRY(SOUNDVOLUME,      REG_DWORD, "soundVolume"            ),
        LOADENTRY(BGMISPAUSED,      REG_DWORD, "bgmIsPaused"            ),
        LOADENTRY(ISSOUNDON,        REG_DWORD, "isSoundOn"              ),
        LOADENTRY(NOCTRL,           REG_DWORD, "m_isNoCtrl"             ),
        LOADENTRY(MAKEMISSEFFECT,   REG_DWORD, "m_bMakeMissEffect"      ),
        LOADENTRY(ISEFFECTON,       REG_DWORD, "isEffectOn"             ),
        LOADENTRY(SNAP,             REG_DWORD, "m_monsterSnapOn_NoSkill"),
        LOADENTRY(ISITEMSNAP,       REG_DWORD, "m_isItemSnap"           ),
        LOADENTRY(SKILLSNAP,        REG_DWORD, "m_monsterSnapOn_Skill"  ),
        LOADENTRY(ISFIXEDCAMERA,    REG_DWORD, "g_isFixedCamera"        ),
        LOADENTRY(ONHOUSERAI,       REG_DWORD, "onHoUserAI"             ),
        LOADENTRY(ONMERUSERAI,      REG_DWORD, "onMerUserAI"            ),
#undef LOADENTRY
    };

    this->Reset();

    if(RegOpenKeyEx(HKEY_GRAVITY, SETTINGS_REGPATH, 0, KEY_READ, &hKey)==ERROR_SUCCESS)
    {
        RegUtilLoad(hKey, LoadInfo, __ARRAYSIZE(LoadInfo), NULL);
        RegCloseKey(hKey);
    }

    if(RegOpenKeyEx(HKEY_GRAVITY, SETTINGS_REGPATH_OPTION, 0, KEY_READ, &hKey)==ERROR_SUCCESS)
    {
        RegUtilLoad(hKey, LoadOptionInfo, __ARRAYSIZE(LoadOptionInfo), NULL);
        RegCloseKey(hKey);
    }
}

void __stdcall CSettingsReg::Reset()
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
    this->Set(SE_PROVIDERNAME,     "No Provider");
    //
    this->Set(SE_SHOWTIPSATSTARTUP,1UL          );
    this->Set(SE_TRILINEARFILTER,  0UL          );
    //
    this->Set(SE_STREAMVOLUME,     100UL        );
    this->Set(SE_SOUNDVOLUME,      100UL        );
    this->Set(SE_MOUSEEXCLUSIVE,   0UL          );
    this->Set(SE_BGMISPAUSED,      0UL          );
    this->Set(SE_ISSOUNDON,        1UL          );
    this->Set(SE_NOTRADE,          0UL          );
    this->Set(SE_NOSHIFT,          0UL          );
    this->Set(SE_NOCTRL,           0UL          );
    this->Set(SE_SKILLFAIL,        0UL          );
    this->Set(SE_NOTALKMSG,        0UL          );
    this->Set(SE_NOTALKMSG2,       0UL          );
    this->Set(SE_SHOWNAME,         0UL          );
    this->Set(SE_AURA,             0UL          );
    this->Set(SE_WINDOW,           0UL          );
    this->Set(SE_MAKEMISSEFFECT,   1UL          );
    this->Set(SE_ISEFFECTON,       1UL          );
    this->Set(SE_SHOPPING,         0UL          );
    this->Set(SE_STATEINFO,        0UL          );
    this->Set(SE_LOGINOUT,         0UL          );
    this->Set(SE_SNAP,             0UL          );
    this->Set(SE_ISITEMSNAP,       0UL          );
    this->Set(SE_SKILLSNAP,        1UL          );
    this->Set(SE_ISFIXEDCAMERA,    0UL          );
    this->Set(SE_ONHOUSERAI,       0UL          );
    this->Set(SE_ONMERUSERAI,      0UL          );
    this->Set(SE_MONSTERHP,        0UL          );
    this->Set(SE_Q1,               0UL          );
    this->Set(SE_Q2,               0UL          );
    //
    this->Set(SF__ALL_FLAGS, false);
}

bool __stdcall CSettingsReg::IsAvail(SETTINGENTRY nEntry)
{
    switch(nEntry)
    {
        case SE_ISFULLSCREENMODE:
        case SE_WIDTH:
        case SE_HEIGHT:
        case SE_BITPERPIXEL:
        case SE_DEVICECNT:
        case SE_MODECNT:
        case SE_ISVOODOO:
        case SE_ISLIGHTMAP:
        case SE_SPRITEMODE:
        case SE_TEXTUREMODE:
        case SE_NUMSAMPLETYPE:
        case SE_FOG:
        case SE_SOUNDMODE:
        case SE_SPEAKERTYPE:
        case SE_DIGITALRATETYPE:
        case SE_DIGITALBITSTYPE:
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
        //case SE_MOUSEEXCLUSIVE:
        case SE_BGMISPAUSED:
        case SE_ISSOUNDON:
        //case SE_NOTRADE:
        //case SE_NOSHIFT:
        case SE_NOCTRL:
        //case SE_SKILLFAIL:
        //case SE_NOTALKMSG:
        //case SE_NOTALKMSG2:
        //case SE_SHOWNAME:
        //case SE_AURA:
        //case SE_WINDOW:
        case SE_MAKEMISSEFFECT:
        case SE_ISEFFECTON:
        //case SE_SHOPPING:
        //case SE_STATEINFO:
        //case SE_LOGINOUT:
        case SE_SNAP:
        case SE_ISITEMSNAP:
        case SE_SKILLSNAP:
        case SE_ISFIXEDCAMERA:
        case SE_ONHOUSERAI:
        case SE_ONMERUSERAI:
        //case SE_MONSTERHP:
        //case SE_Q1:
        //case SE_Q2:
            return true;
    }

    return false;
}

bool __stdcall CSettingsReg::IsAdminRequired()
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
    }

    return false;
}

bool __stdcall CSettingsReg::IsSane()
{
    // always considered sane
    return true;
}

SETTINGENGINEID __stdcall CSettingsReg::GetEngineID()
{
    return SENGINE_REG;
}

void __stdcall CSettingsReg::ResetSettings()
{
    RegUtilDrop(HKEY_GRAVITY, SETTINGS_REGPATH);
}
