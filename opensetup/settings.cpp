// -----------------------------------------------------------------
// RagnarokOnline OpenSetup
// (c) 2010 Ai4rei/AN
// See doc/license.txt for details.
// -----------------------------------------------------------------

#include "snippets/btypes.h"
#include "snippets/cstr.h"
#include "snippets/regutil.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "settings.h"

#define SETTINGS_REGPATH "Software\\Gravity Soft\\Ragnarok"

unsigned long __stdcall CSettings::Get(enum SettingEntry nEntry)
{
#define GETENTRY(name) case SE_##name: return this->##name
    switch(nEntry)
    {
        GETENTRY(ISFULLSCREENMODE);
        GETENTRY(WIDTH           );
        GETENTRY(HEIGHT          );
        GETENTRY(BITPERPIXEL     );
        GETENTRY(DEVICECNT       );
        GETENTRY(MODECNT         );
        GETENTRY(ISVOODOO        );
        GETENTRY(ISLIGHTMAP      );
        GETENTRY(SPRITEMODE      );
        GETENTRY(TEXTUREMODE     );
        GETENTRY(NUMSAMPLETYPE   );
        GETENTRY(FOG             );
        GETENTRY(SOUNDMODE       );
        GETENTRY(SPEAKERTYPE     );
        GETENTRY(DIGITALRATETYPE );
        GETENTRY(DIGITALBITSTYPE );
    }
    DebugBreakHere();
    return 0;
#undef GETENTRY
}

void __stdcall CSettings::Set(enum SettingEntry nEntry, unsigned long luValue)
{
#define SETENTRY(name) case SE_##name: this->##name = luValue; break
    switch(nEntry)
    {
        SETENTRY(ISFULLSCREENMODE);
        SETENTRY(WIDTH           );
        SETENTRY(HEIGHT          );
        SETENTRY(BITPERPIXEL     );
        SETENTRY(DEVICECNT       );
        SETENTRY(MODECNT         );
        SETENTRY(ISVOODOO        );
        SETENTRY(ISLIGHTMAP      );
        SETENTRY(SPRITEMODE      );
        SETENTRY(TEXTUREMODE     );
        SETENTRY(NUMSAMPLETYPE   );
        SETENTRY(FOG             );
        SETENTRY(SOUNDMODE       );
        SETENTRY(SPEAKERTYPE     );
        SETENTRY(DIGITALRATETYPE );
        SETENTRY(DIGITALBITSTYPE );
        default: DebugBreakHere();
    }
#undef SETENTRY
}

void __stdcall CSettings::Set(enum SettingEntry nEntry, GUID* lpGuid)
{
    if(nEntry==SE_GUIDDRIVER)
    {
        CopyMemory(&this->GUIDDRIVER, lpGuid, sizeof(this->GUIDDRIVER));
    }
    else if(nEntry==SE_GUIDDEVICE)
    {
        CopyMemory(&this->GUIDDEVICE, lpGuid, sizeof(this->GUIDDEVICE));
    }
    else
    {
        DebugBreakHere();
    }
}

void __stdcall CSettings::Set(enum SettingEntry nEntry, const char* lpszString)
{
    if(nEntry==SE_DEVICENAME)
    {
        lstrcpyn(this->DEVICENAME, lpszString, sizeof(this->DEVICENAME));
    }
    else if(nEntry==SE_PROVIDERNAME)
    {
        lstrcpyn(this->PROVIDERNAME, lpszString, sizeof(this->PROVIDERNAME));
    }
    else
    {
        DebugBreakHere();
    }
}

void __stdcall CSettings::Save(void)
{
    HKEY hKey;
    LONG lResult;
    struct RegUtilSaveInfo SaveInfo[] =
    {
#define SAVEENTRY(name,type) { #name, &this->##name, sizeof(this->##name), (type) }
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
#undef SAVEENTRY
    };

    // NOTE: HKEY_LOCAL_MACHINE requires UAC requireAdministrator
    //       HKEY_CURRENT_USER  requires UAC asInvoker
#ifndef HKLM_TO_HKCU
    lResult = RegCreateKeyEx(HKEY_LOCAL_MACHINE, SETTINGS_REGPATH, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL);
#else
    lResult = RegCreateKeyEx(HKEY_CURRENT_USER, SETTINGS_REGPATH, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL);
#endif

    if(lResult!=ERROR_SUCCESS)
    {
        MessageBox(NULL, "Failed to save settings. Not enough privileges?", "Error - OpenSetup", MB_OK|MB_ICONSTOP);
        return;
    }

    if(!RegUtilSave(hKey, SaveInfo, ARRAYLEN(SaveInfo)))
    {
        MessageBox(NULL, "Failed to save settings. Not enough privileges?", "Error - OpenSetup", MB_OK|MB_ICONSTOP);
    }

    RegFlushKey(hKey);
    RegCloseKey(hKey);
}

void __stdcall CSettings::Load(void)
{
    unsigned long luDeviceNameLen, luGUIDDeviceLen, luGUIDDriverLen, luProviderNameLen;
    HKEY hKey;
    struct RegUtilLoadInfo LoadInfo[] =
    {
#define LOADENTRY(name,type) { #name, &this->##name, sizeof(this->##name), NULL, (type) }
        LOADENTRY(ISFULLSCREENMODE, REG_DWORD ),
        LOADENTRY(WIDTH,            REG_DWORD ),
        LOADENTRY(HEIGHT,           REG_DWORD ),
        LOADENTRY(BITPERPIXEL,      REG_DWORD ),
        LOADENTRY(DEVICECNT,        REG_DWORD ),
        LOADENTRY(MODECNT,          REG_DWORD ),
        LOADENTRY(ISVOODOO,         REG_DWORD ),
        LOADENTRY(ISLIGHTMAP,       REG_DWORD ),
        LOADENTRY(SPRITEMODE,       REG_DWORD ),
        LOADENTRY(TEXTUREMODE,      REG_DWORD ),
        LOADENTRY(NUMSAMPLETYPE,    REG_DWORD ),
        LOADENTRY(FOG,              REG_DWORD ),
        LOADENTRY(SOUNDMODE,        REG_DWORD ),
        LOADENTRY(SPEAKERTYPE,      REG_DWORD ),
        LOADENTRY(DIGITALRATETYPE,  REG_DWORD ),
        LOADENTRY(DIGITALBITSTYPE,  REG_DWORD ),
        LOADENTRY(GUIDDRIVER,       REG_BINARY),
        LOADENTRY(GUIDDEVICE,       REG_BINARY),
        LOADENTRY(DEVICENAME,       REG_SZ    ),
        LOADENTRY(PROVIDERNAME,     REG_SZ    ),
#undef LOADENTRY
    };

    // Length Info
    LoadInfo[SE_DEVICENAME].lpluValueLength = &luDeviceNameLen;
    LoadInfo[SE_GUIDDEVICE].lpluValueLength = &luGUIDDeviceLen;
    LoadInfo[SE_GUIDDRIVER].lpluValueLength = &luGUIDDriverLen;
    LoadInfo[SE_PROVIDERNAME].lpluValueLength = &luProviderNameLen;

    this->Reset();

    if(RegOpenKeyEx(HKEY_LOCAL_MACHINE, SETTINGS_REGPATH, 0, KEY_READ, &hKey)==ERROR_SUCCESS)
    {
        RegUtilLoad(hKey, LoadInfo, ARRAYLEN(LoadInfo));
        RegCloseKey(hKey);
    }
}

void __stdcall CSettings::Reset(void)
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
}
