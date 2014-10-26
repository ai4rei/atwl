// -----------------------------------------------------------------
// RagnarokOnline OpenSetup
// (c) 2010-2014 Ai4rei/AN
// See doc/license.txt for details.
//
// -----------------------------------------------------------------

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>

#include <regutil.h>

#include "error.h"
#include "opensetup.h"
#include "resource.h"
#include "settings.h"

#define SETTINGS_REGPATH "Software\\Gravity Soft\\Ragnarok"
#define SETTINGS_REGPATH_UIRECTINFO SETTINGS_REGPATH"\\UIRectInfo"
#define SETTINGS_REGPATH_SKILLUSELEVELINFO SETTINGS_REGPATH"\\SkillUseLevelInfo"

static const char* l_lppszUserDataItems[] =
{
    "_tmpEmblem",
    "Chat",
    "Chat_BM",
    "Replay",
    "ScreenShot",
};

// shared section for elevated actions
#pragma data_seg(".shared")
static BOOL l_bAchievementUnlocked = FALSE;  // signal valid data in l_IPCBuffer
static unsigned long l_luHash = 0;
static SETTINGSENTRIES l_IPCBuffer = { 0 };
static int l_nIPCFlags = 0;
static LONG l_lSpinLock = 0;
#pragma data_seg()
#pragma comment(linker, "/SECTION:.shared,RWS")

void __stdcall CSettings::DropFolders(const char* lpszFolders)
{
    // we do not have the guts to delete everything immediately, so
    // let's delete to recycle bin, if any.
    SHFILEOPSTRUCTA Fo = { 0 };

    Fo.wFunc = FO_DELETE;
    Fo.pFrom = lpszFolders;
    Fo.fFlags = FOF_ALLOWUNDO|FOF_NOCONFIRMATION|FOF_NOERRORUI|FOF_WANTNUKEWARNING;

    SHFileOperationA(&Fo);
}

void __stdcall CSettings::DropFolderList(const char** lppszList, unsigned long luItems)
{
    char* lpszSlash;
    char* lpszItems;
    char szBasePath[MAX_PATH], szFilePath[MAX_PATH];
    unsigned long luIdx, luItemLen = 0, luLen;

    GetModuleFileNameA(NULL, szBasePath, __ARRAYSIZE(szBasePath));

    if((lpszSlash = strrchr(szBasePath, '\\'))!=NULL)
    {
        lpszSlash[1] = 0;
        luLen = lstrlenA(szBasePath);

        for(luIdx = 0; luIdx<luItems; luIdx++)
        {
            luItemLen+= luLen+lstrlenA(lppszList[luIdx])+1;  // +1 = account for zero-termination
        }

        luItemLen++;  // account for array termination

        lpszItems = new char[luItemLen];
        luLen = 0;

        for(luIdx = 0; luIdx<luItems; luIdx++)
        {
            wsprintfA(szFilePath, "%s%s", szBasePath, lppszList[luIdx]);

            if(GetFileAttributes(szFilePath)!=~0UL)
            {// add it only if it exists
                luLen+= wsprintfA(lpszItems+luLen, "%s", szFilePath)+1;
            }
        }

        if(luLen)
        {// there is something to delete
            lpszItems[luLen] = 0;  // terminate array

            /* this:: */DropFolders(lpszItems);
        }

        delete[] lpszItems;
    }
}

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
        GETENTRY(SKILLSNAP        );
        GETENTRY(ISFIXEDCAMERA    );
        GETENTRY(ONHOUSERAI       );
        GETENTRY(ONMERUSERAI      );
        GETENTRY(MONSTERHP        );
        GETENTRY(Q1               );
        GETENTRY(Q2               );
    }
    DebugBreakHere();
    return 0;
#undef GETENTRY
}

void __stdcall CSettings::Get(SETTINGENTRY nEntry, GUID* lpGuid)
{
#define GETENTRY(name) case SE_##name: CopyMemory(lpGuid, &this->m_Entries.##name, sizeof(lpGuid[0])); break
    switch(nEntry)
    {
        GETENTRY(GUIDDRIVER       );
        GETENTRY(GUIDDEVICE       );
        default: DebugBreakHere();
    }
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
        SETENTRY(SKILLSNAP        );
        SETENTRY(ISFIXEDCAMERA    );
        SETENTRY(ONHOUSERAI       );
        SETENTRY(ONMERUSERAI      );
        SETENTRY(MONSTERHP        );
        SETENTRY(Q1               );
        SETENTRY(Q2               );
        default: DebugBreakHere();
    }
#undef SETENTRY
}

void __stdcall CSettings::Set(SETTINGENTRY nEntry, const GUID* lpGuid)
{
#define SETENTRY(name) case SE_##name: CopyMemory(&this->m_Entries.##name, lpGuid, sizeof(this->m_Entries.##name)); break
    switch(nEntry)
    {
        SETENTRY(GUIDDRIVER       );
        SETENTRY(GUIDDEVICE       );
        default: DebugBreakHere();
    }
#undef SETENTRY
}

void __stdcall CSettings::Set(SETTINGENTRY nEntry, const char* lpszString)
{
#define SETENTRY(name) case SE_##name: lstrcpynA(this->m_Entries.##name, lpszString, sizeof(this->m_Entries.##name)); break
    switch(nEntry)
    {
        SETENTRY(DEVICENAME       );
        SETENTRY(PROVIDERNAME     );
        default: DebugBreakHere();
    }
#undef SETENTRY
}

void __stdcall CSettings::Set(SETTINGFLAG nFlag, bool bState)
{
    if(bState)
    {
        this->m_nFlags|= nFlag;
    }
    else
    {
        this->m_nFlags&=~nFlag;
    }
}

unsigned long __stdcall CSettings::SaveToIPC()
{
    EnterSpinLock(&l_lSpinLock);

    CopyMemory(&l_IPCBuffer, &this->m_Entries, sizeof(l_IPCBuffer));
    l_nIPCFlags = this->m_nFlags;

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
        this->m_nFlags = l_nIPCFlags;

        l_bAchievementUnlocked = FALSE;
        l_luHash = 0;
    }
    else
    {
        DebugBreakHere();
    }

    LeaveSpinLock(&l_lSpinLock);
}

void __stdcall CSettings::ResetUI()
{
    RegUtilDrop(HKEY_GRAVITY, SETTINGS_REGPATH_UIRECTINFO);
}

void __stdcall CSettings::ResetSkillLevel()
{
    RegUtilDrop(HKEY_GRAVITY, SETTINGS_REGPATH_SKILLUSELEVELINFO);
}

void __stdcall CSettings::ResetUserData()
{
    /* this:: */DropFolderList(l_lppszUserDataItems, __ARRAYSIZE(l_lppszUserDataItems));
}

void __stdcall CSettings::ResetInstall()
{
    char szPathName[MAX_PATH];
    char* lpszSlash;
    HKEY hKey;
    LONG lResult;

    if(!GetModuleFileName(NULL, szPathName, __ARRAYSIZE(szPathName)) || (lpszSlash = strrchr(szPathName, '\\'))==NULL)
    {
        return;
    }
    lpszSlash[1] = 0;  // keep the backslash

    REGUTILSAVEINFO SaveInfo[] =
    {
        { NULL,         szPathName, lpszSlash-szPathName+2, REG_SZ },  // @
        { "RagPath",    szPathName, lpszSlash-szPathName+2, REG_SZ },
        { "SakrayPath", szPathName, lpszSlash-szPathName+2, REG_SZ },
    };

    lResult = RegCreateKeyEx(HKEY_CURRENT_USER, "Software\\Gravity\\RagnarokOnline", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, NULL);

    if(lResult!=ERROR_SUCCESS)
    {
        SetLastError(lResult);
        CError::ErrorMessage(NULL, TEXT_ERROR_HKEY_CREATE);
        return;
    }

    if(!RegUtilSave(hKey, SaveInfo, __ARRAYSIZE(SaveInfo), &lResult))
    {
        SetLastError(lResult);
        CError::ErrorMessage(NULL, TEXT_ERROR_HKEY_WRITE);
    }

    RegFlushKey(hKey);
    RegCloseKey(hKey);
}
