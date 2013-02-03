// -----------------------------------------------------------------
// RagnarokOnline OpenSetup
// (c) 2010-2013 Ai4rei/AN
// See doc/license.txt for details.
//
// -----------------------------------------------------------------

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "opensetup.h"
#include "settings.h"

// shared section for elevated actions
#pragma data_seg(".shared")
static BOOL l_bAchievementUnlocked = FALSE;  // signal valid data in l_IPCBuffer
static unsigned long l_luHash = 0;
static SETTINGSENTRIES l_IPCBuffer = { 0 };
static LONG l_lSpinLock = 0;
#pragma data_seg()
#pragma comment(linker, "/SECTION:.shared,RWS")

unsigned long __stdcall CSettings::Get(SETTINGENTRY nEntry)
{
#define GETENTRY(name) case SE_##name: return this->m_Entries.##name
    switch(nEntry)
    {
        GETENTRY(ISFULLSCREENMODE );
        GETENTRY(WIDTH            );
        GETENTRY(HEIGHT           );
        GETENTRY(BITPERPIXEL      );
        GETENTRY(DEVICECNT        );
        GETENTRY(MODECNT          );
        GETENTRY(ISVOODOO         );
        GETENTRY(ISLIGHTMAP       );
        GETENTRY(SPRITEMODE       );
        GETENTRY(TEXTUREMODE      );
        GETENTRY(NUMSAMPLETYPE    );
        GETENTRY(FOG              );
        GETENTRY(SOUNDMODE        );
        GETENTRY(SPEAKERTYPE      );
        GETENTRY(DIGITALRATETYPE  );
        GETENTRY(DIGITALBITSTYPE  );
        GETENTRY(SHOWTIPSATSTARTUP);
        GETENTRY(TRILINEARFILTER  );
        GETENTRY(STREAMVOLUME     );
        GETENTRY(SOUNDVOLUME      );
        GETENTRY(MOUSEEXCLUSIVE   );
        GETENTRY(BGMISPAUSED      );
        GETENTRY(ISSOUNDON        );
        GETENTRY(NOTRADE          );
        GETENTRY(NOSHIFT          );
        GETENTRY(NOCTRL           );
        GETENTRY(SKILLFAIL        );
        GETENTRY(NOTALKMSG        );
        GETENTRY(NOTALKMSG2       );
        GETENTRY(SHOWNAME         );
        GETENTRY(AURA             );
        GETENTRY(WINDOW           );
        GETENTRY(MAKEMISSEFFECT   );
        GETENTRY(ISEFFECTON       );
        GETENTRY(SHOPPING         );
        GETENTRY(STATEINFO        );
        GETENTRY(LOGINOUT         );
        GETENTRY(SNAP             );
        GETENTRY(ISITEMSNAP       );
        GETENTRY(ISFIXEDCAMERA    );
        GETENTRY(ONHOUSERAI       );
        GETENTRY(ONMERUSERAI      );
    }
    DebugBreakHere();
    return 0;
#undef GETENTRY
}

void __stdcall CSettings::Set(SETTINGENTRY nEntry, unsigned long luValue)
{
#define SETENTRY(name) case SE_##name: this->m_Entries.##name = luValue; break
    switch(nEntry)
    {
        SETENTRY(ISFULLSCREENMODE );
        SETENTRY(WIDTH            );
        SETENTRY(HEIGHT           );
        SETENTRY(BITPERPIXEL      );
        SETENTRY(DEVICECNT        );
        SETENTRY(MODECNT          );
        SETENTRY(ISVOODOO         );
        SETENTRY(ISLIGHTMAP       );
        SETENTRY(SPRITEMODE       );
        SETENTRY(TEXTUREMODE      );
        SETENTRY(NUMSAMPLETYPE    );
        SETENTRY(FOG              );
        SETENTRY(SOUNDMODE        );
        SETENTRY(SPEAKERTYPE      );
        SETENTRY(DIGITALRATETYPE  );
        SETENTRY(DIGITALBITSTYPE  );
        SETENTRY(SHOWTIPSATSTARTUP);
        SETENTRY(TRILINEARFILTER  );
        SETENTRY(STREAMVOLUME     );
        SETENTRY(SOUNDVOLUME      );
        SETENTRY(MOUSEEXCLUSIVE   );
        SETENTRY(BGMISPAUSED      );
        SETENTRY(ISSOUNDON        );
        SETENTRY(NOTRADE          );
        SETENTRY(NOSHIFT          );
        SETENTRY(NOCTRL           );
        SETENTRY(SKILLFAIL        );
        SETENTRY(NOTALKMSG        );
        SETENTRY(NOTALKMSG2       );
        SETENTRY(SHOWNAME         );
        SETENTRY(AURA             );
        SETENTRY(WINDOW           );
        SETENTRY(MAKEMISSEFFECT   );
        SETENTRY(ISEFFECTON       );
        SETENTRY(SHOPPING         );
        SETENTRY(STATEINFO        );
        SETENTRY(LOGINOUT         );
        SETENTRY(SNAP             );
        SETENTRY(ISITEMSNAP       );
        SETENTRY(ISFIXEDCAMERA    );
        SETENTRY(ONHOUSERAI       );
        SETENTRY(ONMERUSERAI      );
        default: DebugBreakHere();
    }
#undef SETENTRY
}

void __stdcall CSettings::Set(SETTINGENTRY nEntry, GUID* lpGuid)
{
    if(nEntry==SE_GUIDDRIVER)
    {
        CopyMemory(&this->m_Entries.GUIDDRIVER, lpGuid, sizeof(this->m_Entries.GUIDDRIVER));
    }
    else if(nEntry==SE_GUIDDEVICE)
    {
        CopyMemory(&this->m_Entries.GUIDDEVICE, lpGuid, sizeof(this->m_Entries.GUIDDEVICE));
    }
    else
    {
        DebugBreakHere();
    }
}

void __stdcall CSettings::Set(SETTINGENTRY nEntry, const char* lpszString)
{
    if(nEntry==SE_DEVICENAME)
    {
        lstrcpyn(this->m_Entries.DEVICENAME, lpszString, sizeof(this->m_Entries.DEVICENAME));
    }
    else if(nEntry==SE_PROVIDERNAME)
    {
        lstrcpyn(this->m_Entries.PROVIDERNAME, lpszString, sizeof(this->m_Entries.PROVIDERNAME));
    }
    else
    {
        DebugBreakHere();
    }
}

unsigned long __stdcall CSettings::SaveToIPC(void)
{
    EnterSpinLock(&l_lSpinLock);

    CopyMemory(&l_IPCBuffer, &this->m_Entries, sizeof(l_IPCBuffer));

    l_bAchievementUnlocked = TRUE;
    l_luHash = GetTickCount();

    LeaveSpinLock(&l_lSpinLock);
    return l_luHash;
}

void __stdcall CSettings::LoadFromIPC(unsigned long luHash)
{
    EnterSpinLock(&l_lSpinLock);

    if(l_bAchievementUnlocked && l_luHash==luHash)
    {
        CopyMemory(&this->m_Entries, &l_IPCBuffer, sizeof(this->m_Entries));

        l_bAchievementUnlocked = FALSE;
        l_luHash = 0;
    }
    else
    {
        DebugBreakHere();
    }

    LeaveSpinLock(&l_lSpinLock);
}
