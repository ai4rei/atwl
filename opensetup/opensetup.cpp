// -----------------------------------------------------------------
// RagnarokOnline OpenSetup
// (c) 2010+ Ai4rei/AN
// See doc/license.txt for details.
//
// -----------------------------------------------------------------

#include <windows.h>
#include <commctrl.h>
#include <stdio.h>

#include <dx7enum.h>
#include <regutil.h>

#include "config.h"
#include "error.h"
#include "log.h"
#include "parentctrl.h"
#include "resource.h"
#include "roext.h"
#include "settings.h"
#include "settings_lua.h"
#include "settings_reg.h"
#include "sound.h"
#include "tab.h"
#include "ui.h"

#include "opensetup.h"

static const char* l_lpszSignature = "$ RagnarokOnline OpenSetup, "APP_VERSION", build on "__DATE__" @ "__TIME__", (c) 2010-2017 Ai4rei/AN $";

static const unsigned int l_lpuNumSampleTypes[] =
{
    TEXT_NUMSAMPLETYPE_48CHANNEL,
    TEXT_NUMSAMPLETYPE_32CHANNEL,
    TEXT_NUMSAMPLETYPE_16CHANNEL,
};
static const unsigned int l_lpuSoundModes[] =
{
    TEXT_SOUNDMODE_NOSOUND,
    TEXT_SOUNDMODE_2DSOUND,
    TEXT_SOUNDMODE_3DSOUND,
};
static const unsigned int l_lpuSpeakerTypes[] =
{
    TEXT_SPEAKERTYPE_2SPEAKER,
    TEXT_SPEAKERTYPE_HEADPHONE,
    TEXT_SPEAKERTYPE_SURROUND,
    TEXT_SPEAKERTYPE_4SPEAKER,
};
static const unsigned int l_lpuDigitalRateTypes[] =
{
    TEXT_DIGITALRATETYPE_22K,
    TEXT_DIGITALRATETYPE_11K,
    TEXT_DIGITALRATETYPE_8K,
};
static const unsigned int l_lpuDigitalBitsTypes[] =
{
    TEXT_DIGITALBITSTYPE_16BIT,
    TEXT_DIGITALBITSTYPE_8BIT,
};

// registry target (global)
HKEY HKEY_GRAVITY = HKEY_LOCAL_MACHINE;

static BOOL l_LoadDefaults = FALSE;
static HFONT l_DlgFont = NULL;
static DX7EDISPLAYDRIVERINFO l_DI = { 0 };
static CParentCtrl ParentCtrl;
static CROExt ROExt;
static CTabMgr TabMgr;

static CSettingsLua SettingsLua;
static CSettingsReg SettingsReg;
static CSettings* Settings = NULL;

static inline unsigned long __stdcall GetDriverIndex(unsigned long luComboIndex)
{
    unsigned long luDriver, luDevice;

    for(luDriver = 0; luDriver<l_DI.luDrivers; luDriver++)
    {
        for(luDevice = 0; luDevice<l_DI.Driver[luDriver].luDevicesHW; luDevice++)
        {
            if(!luComboIndex)
            {
                return luDriver;
            }

            luComboIndex--;
        }
    }

    return 0;
}

static inline unsigned long __stdcall GetDeviceIndex(unsigned long luComboIndex)
{
    unsigned long luDriver, luDevice;

    for(luDriver = 0; luDriver<l_DI.luDrivers; luDriver++)
    {
        for(luDevice = 0; luDevice<l_DI.Driver[luDriver].luDevicesHW; luDevice++)
        {
            if(!luComboIndex)
            {
                return luDevice;
            }

            luComboIndex--;
        }
    }

    return 0;
}

static inline void __stdcall DisableUnavailableSetting(SETTINGENTRY nEntry, HWND hParent, int nControlId)
{
    EnableWindow(GetDlgItem(hParent, nControlId), Settings->IsAvail(nEntry));
}

static void __stdcall OnChangeDevice(HWND hWnd)
{
    char szLabel[128];
    int nResult;
    unsigned long luMode, luDriver;
    LPCDX7EDISPLAYDRIVER lpDriver;
    HWND hChild;

    luDriver = GetDriverIndex(Settings->Get(SE_DEVICECNT));

    if(luDriver>=l_DI.luDrivers)
    {// ignore
        return;
    }

    lpDriver = &l_DI.Driver[luDriver];

    // save selection
    hChild  = GetDlgItem(hWnd, IDCOMBOBOX_RESOLUTION);
    nResult = SendMessage(hChild, CB_GETCURSEL, 0, 0);

    // empty combobox
    SendMessage(hChild, CB_RESETCONTENT, 0, 0);

    for(luMode = 0; luMode<lpDriver->luModes; luMode++)
    {
        wsprintfA(szLabel, "%lu x %lu x %lu", lpDriver->Mode[luMode].luWidth, lpDriver->Mode[luMode].luHeight, lpDriver->Mode[luMode].luBitDepth);

        SendMessage(hChild, CB_ADDSTRING, 0, (LPARAM)szLabel);
    }

    // restore selection
    if(SendMessage(hChild, CB_SETCURSEL, nResult, 0)==CB_ERR)
    {// overflow
        SendMessage(hChild, CB_SETCURSEL, 0, 0);
    }
}

static inline bool __stdcall IsZeroGUID(REFGUID rguid)
{
    static const GUID zguid = { 0 };

    return IsEqualGUID(rguid, zguid)!=FALSE;
}

static void __stdcall OnUpdateDevice(HWND hWnd)
{
    int nResult;
    unsigned long luDriver, luDevice;
    HWND hChild;

    // save selection
    hChild  = GetDlgItem(hWnd, IDCOMBOBOX_VIDEODEVICE);
    nResult = SendMessage(hChild, CB_GETCURSEL, 0, 0);

    // empty combobox
    SendMessage(hChild, CB_RESETCONTENT, 0, 0);

    // feed the list
    for(luDriver = 0; luDriver<l_DI.luDrivers; luDriver++)
    {
        bool bIsPrimary = IsZeroGUID(l_DI.Driver[luDriver].DriverGuid);
        char szDisplayName[86];

        if(UI::GetCheckBoxTick(hWnd, IDCHECKBOX_HELDEVICE))
        {// force software rendering
            for(luDevice = 0; luDevice<l_DI.Driver[luDriver].luDevicesSW; luDevice++)
            {
                wsprintfA(szDisplayName, "%s [%s]", l_DI.Driver[luDriver].szName, l_DI.Driver[luDriver].DeviceSW[luDevice].szName);

                SendMessage(hChild, CB_ADDSTRING, 0, (LPARAM)(bIsPrimary ? l_DI.Driver[luDriver].DeviceSW[luDevice].szName : szDisplayName));
            }
        }
        else
        {
            for(luDevice = 0; luDevice<l_DI.Driver[luDriver].luDevicesHW; luDevice++)
            {
                wsprintfA(szDisplayName, "%s [%s]", l_DI.Driver[luDriver].szName, l_DI.Driver[luDriver].DeviceHW[luDevice].szName);

                SendMessage(hChild, CB_ADDSTRING, 0, (LPARAM)(bIsPrimary ? l_DI.Driver[luDriver].DeviceHW[luDevice].szName : szDisplayName));
            }
        }
    }

    // restore selection
    if(SendMessage(hChild, CB_SETCURSEL, nResult, 0)==CB_ERR)
    {// overflow
        SendMessage(hChild, CB_SETCURSEL, 0, 0);
    }

    // refresh resolution
    OnChangeDevice(hWnd);
}

static void __stdcall GetVideoTab(HWND hWnd)
{
    int nComboIdx;

    // complex combo boxes
    if((nComboIdx = SendMessage(GetDlgItem(hWnd, IDCOMBOBOX_VIDEODEVICE), CB_GETCURSEL, 0, 0))!=CB_ERR)
    {
        unsigned long luDriver = GetDriverIndex(nComboIdx);
        unsigned long luDevice = GetDeviceIndex(nComboIdx);

        Settings->Set(SE_DEVICECNT, nComboIdx);
        Settings->Set(SE_GUIDDRIVER, &l_DI.Driver[luDriver].DriverGuid);

        if(UI::GetCheckBoxTick(hWnd, IDCHECKBOX_HELDEVICE))
        {// force software rendering
            Settings->Set(SE_GUIDDEVICE, DX7E_DeviceType2Guid(l_DI.Driver[luDriver].DeviceSW[luDevice].nType));
            Settings->Set(SE_DEVICENAME, l_DI.Driver[luDriver].DeviceSW[luDevice].szName);
        }
        else
        {
            Settings->Set(SE_GUIDDEVICE, DX7E_DeviceType2Guid(l_DI.Driver[luDriver].DeviceHW[luDevice].nType));
            Settings->Set(SE_DEVICENAME, l_DI.Driver[luDriver].DeviceHW[luDevice].szName);
        }
    }
    if((nComboIdx = SendMessage(GetDlgItem(hWnd, IDCOMBOBOX_RESOLUTION), CB_GETCURSEL, 0, 0))!=CB_ERR)
    {
        unsigned long luDriver = GetDriverIndex(Settings->Get(SE_DEVICECNT));

        Settings->Set(SE_MODECNT,     nComboIdx);
        Settings->Set(SE_WIDTH,       l_DI.Driver[luDriver].Mode[nComboIdx].luWidth   );
        Settings->Set(SE_HEIGHT,      l_DI.Driver[luDriver].Mode[nComboIdx].luHeight  );
        Settings->Set(SE_BITPERPIXEL, l_DI.Driver[luDriver].Mode[nComboIdx].luBitDepth);
    }

    // check boxes
    Settings->Set(SE_ISFULLSCREENMODE, UI::GetCheckBoxTick(hWnd, IDCHECKBOX_FULLSCREEN)     );
    Settings->Set(SE_FOG,              UI::GetCheckBoxTick(hWnd, IDCHECKBOX_FOG)            );
    Settings->Set(SE_ISLIGHTMAP,       UI::GetCheckBoxTick(hWnd, IDCHECKBOX_LIGHTMAP)       );
    Settings->Set(SE_TRILINEARFILTER,  UI::GetCheckBoxTick(hWnd, IDCHECKBOX_TRILINEARFILTER));
    Settings->Set(SE_ISVOODOO,         UI::GetCheckBoxTick(hWnd, IDCHECKBOX_VOODOO)         );
    Settings->Set(SE_MOUSEEXCLUSIVE,   UI::GetCheckBoxTick(hWnd, IDCHECKBOX_MOUSEEXCLUSIVE) );

    // track bars
    Settings->Set(SE_SPRITEMODE,       SendMessage(GetDlgItem(hWnd, IDTRACKBAR_SPRITEQ),    TBM_GETPOS,  0, 0));
    Settings->Set(SE_TEXTUREMODE,      SendMessage(GetDlgItem(hWnd, IDTRACKBAR_TEXTUREQ),   TBM_GETPOS,  0, 0));
}

static void __stdcall GetSoundTab(HWND hWnd)
{
    int nComboIdx;

    // simple combo boxes
    if((nComboIdx = SendMessage(GetDlgItem(hWnd, IDCOMBOBOX_SOUND_MODE), CB_GETCURSEL, 0, 0))!=CB_ERR)
    {
        Settings->Set(SE_SOUNDMODE, nComboIdx);
    }
    if((nComboIdx = SendMessage(GetDlgItem(hWnd, IDCOMBOBOX_SOUND_SPEAKERTYPE), CB_GETCURSEL, 0, 0))!=CB_ERR)
    {
        Settings->Set(SE_SPEAKERTYPE, nComboIdx);
    }
    if((nComboIdx = SendMessage(GetDlgItem(hWnd, IDCOMBOBOX_SOUND_BITRATE), CB_GETCURSEL, 0, 0))!=CB_ERR)
    {
        Settings->Set(SE_DIGITALRATETYPE, nComboIdx);
    }
    if((nComboIdx = SendMessage(GetDlgItem(hWnd, IDCOMBOBOX_SOUND_BITDEPTH), CB_GETCURSEL, 0, 0))!=CB_ERR)
    {
        Settings->Set(SE_DIGITALBITSTYPE, nComboIdx);
    }
    if((nComboIdx = SendMessage(GetDlgItem(hWnd, IDCOMBOBOX_SOUND_CHANNELS), CB_GETCURSEL, 0, 0))!=CB_ERR)
    {
        Settings->Set(SE_NUMSAMPLETYPE, nComboIdx);
    }

    // check boxes
    Settings->Set(SE_BGMISPAUSED, UI::GetCheckBoxTick(hWnd, IDCHECKBOX_BGMISPAUSED));
    Settings->Set(SE_ISSOUNDON,  !UI::GetCheckBoxTick(hWnd, IDCHECKBOX_ISSOUNDON)  );

    // track bars
    Settings->Set(SE_STREAMVOLUME, SendMessage(GetDlgItem(hWnd, IDTRACKBAR_STREAMVOLUME), TBM_GETPOS,  0, 0));
    Settings->Set(SE_SOUNDVOLUME,  SendMessage(GetDlgItem(hWnd, IDTRACKBAR_SOUNDVOLUME),  TBM_GETPOS,  0, 0));
}

static void __stdcall GetSetupTab(HWND hWnd)
{
    // check boxes
    Settings->Set(SE_AURA,              UI::GetCheckBoxTick(hWnd, IDCHECKBOX_AURA)             );
    Settings->Set(SE_ISFIXEDCAMERA,     UI::GetCheckBoxTick(hWnd, IDCHECKBOX_ISFIXEDCAMERA)    );
    Settings->Set(SE_ISEFFECTON,        UI::GetCheckBoxTick(hWnd, IDCHECKBOX_ISEFFECTON)       );
    Settings->Set(SE_ONHOUSERAI,        UI::GetCheckBoxTick(hWnd, IDCHECKBOX_ONHOUSERAI)       );
    Settings->Set(SE_ISITEMSNAP,        UI::GetCheckBoxTick(hWnd, IDCHECKBOX_ISITEMSNAP)       );
    Settings->Set(SE_LOGINOUT,          UI::GetCheckBoxTick(hWnd, IDCHECKBOX_LOGINOUT)         );
    Settings->Set(SE_ONMERUSERAI,       UI::GetCheckBoxTick(hWnd, IDCHECKBOX_ONMERUSERAI)      );
    Settings->Set(SE_MAKEMISSEFFECT,    UI::GetCheckBoxTick(hWnd, IDCHECKBOX_MAKEMISSEFFECT)   );
    Settings->Set(SE_MONSTERHP,         UI::GetCheckBoxTick(hWnd, IDCHECKBOX_MONSTERHP)        );
    Settings->Set(SE_NOCTRL,            UI::GetCheckBoxTick(hWnd, IDCHECKBOX_NOCTRL)           );
    Settings->Set(SE_NOSHIFT,           UI::GetCheckBoxTick(hWnd, IDCHECKBOX_NOSHIFT)          );
    Settings->Set(SE_NOTRADE,           UI::GetCheckBoxTick(hWnd, IDCHECKBOX_NOTRADE)          );
    Settings->Set(SE_SHOPPING,          UI::GetCheckBoxTick(hWnd, IDCHECKBOX_SHOPPING)         );
    Settings->Set(SE_SNAP,              UI::GetCheckBoxTick(hWnd, IDCHECKBOX_SNAP)             );
    Settings->Set(SE_SKILLFAIL,         UI::GetCheckBoxTick(hWnd, IDCHECKBOX_SKILLFAIL)        );
    Settings->Set(SE_NOTALKMSG,         UI::GetCheckBoxTick(hWnd, IDCHECKBOX_NOTALKMSG)        );
    Settings->Set(SE_NOTALKMSG2,        UI::GetCheckBoxTick(hWnd, IDCHECKBOX_NOTALKMSG2)       );
    Settings->Set(SE_Q1,                UI::GetCheckBoxTick(hWnd, IDCHECKBOX_Q1)               );
    Settings->Set(SE_Q2,                UI::GetCheckBoxTick(hWnd, IDCHECKBOX_Q2)               );
    Settings->Set(SE_SHOWNAME,          UI::GetCheckBoxTick(hWnd, IDCHECKBOX_SHOWNAME)         );
    Settings->Set(SE_STATEINFO,         UI::GetCheckBoxTick(hWnd, IDCHECKBOX_STATEINFO)        );
    Settings->Set(SE_SHOWTIPSATSTARTUP, UI::GetCheckBoxTick(hWnd, IDCHECKBOX_SHOWTIPSATSTARTUP));
    Settings->Set(SE_WINDOW,            UI::GetCheckBoxTick(hWnd, IDCHECKBOX_WINDOW)           );
    Settings->Set(SE_SKILLSNAP,         UI::GetCheckBoxTick(hWnd, IDCHECKBOX_SKILLSNAP)        );
    Settings->Set(SF_RESET_UI,          UI::GetCheckBoxTick(hWnd, IDCHECKBOX_RESETWINDOWS)     );
    Settings->Set(SF_RESET_SKILLLEVEL,  UI::GetCheckBoxTick(hWnd, IDCHECKBOX_RESETSKILLLV)     );
    Settings->Set(SF_RESET_USERDATA,    UI::GetCheckBoxTick(hWnd, IDCHECKBOX_RESETFOLDER)      );
    Settings->Set(SF_RESET_SETTING,     UI::GetCheckBoxTick(hWnd, IDCHECKBOX_RESETSETTING)     );
}

static void __stdcall SetVideoTab(HWND hWnd)
{
    GUID DeviceGuid;

    // software renderer selected?
    Settings->Get(SE_GUIDDEVICE, &DeviceGuid);
    SendMessage(GetDlgItem(hWnd, IDCHECKBOX_HELDEVICE), BM_SETCHECK, (WPARAM)IsEqualGUID(DeviceGuid, DX7E_DeviceType2Guid(DX7E_RDT_HEL)[0]), 0);

    // refresh device and resolution
    OnUpdateDevice(hWnd);

    // initialize track bars
    SendMessage(GetDlgItem(hWnd, IDTRACKBAR_SPRITEQ),  TBM_SETRANGE, (WPARAM)FALSE, (LPARAM)MAKELONG(0, 2));
    SendMessage(GetDlgItem(hWnd, IDTRACKBAR_TEXTUREQ), TBM_SETRANGE, (WPARAM)FALSE, (LPARAM)MAKELONG(0, 2));

    // reflect settings
    UI::BATCHLIST BatchList[] =
    {
        // initialize check boxes
        { IDCHECKBOX_FULLSCREEN,     BM_SETCHECK,  (WPARAM)Settings->Get(SE_ISFULLSCREENMODE), 0                                    },
        { IDCHECKBOX_FOG,            BM_SETCHECK,  (WPARAM)Settings->Get(SE_FOG),              0                                    },
        { IDCHECKBOX_LIGHTMAP,       BM_SETCHECK,  (WPARAM)Settings->Get(SE_ISLIGHTMAP),       0                                    },
        { IDCHECKBOX_TRILINEARFILTER,BM_SETCHECK,  (WPARAM)Settings->Get(SE_TRILINEARFILTER),  0                                    },
        { IDCHECKBOX_VOODOO,         BM_SETCHECK,  (WPARAM)Settings->Get(SE_ISVOODOO),         0                                    },
        { IDCHECKBOX_MOUSEEXCLUSIVE, BM_SETCHECK,  (WPARAM)Settings->Get(SE_MOUSEEXCLUSIVE),   0                                    },
        // initialize track bars
        { IDTRACKBAR_SPRITEQ,        TBM_SETPOS,   (WPARAM)TRUE,                              (LPARAM)Settings->Get(SE_SPRITEMODE)  },
        { IDTRACKBAR_TEXTUREQ,       TBM_SETPOS,   (WPARAM)TRUE,                              (LPARAM)Settings->Get(SE_TEXTUREMODE) },
        // initialize combo boxes
        { IDCOMBOBOX_VIDEODEVICE,    CB_SETCURSEL, (WPARAM)Settings->Get(SE_DEVICECNT),        0                                    },
        { IDCOMBOBOX_RESOLUTION,     CB_SETCURSEL, (WPARAM)Settings->Get(SE_MODECNT),          0                                    },
    };
    UI::BatchMessage(hWnd, BatchList, __ARRAYSIZE(BatchList));

    // store availability
    DisableUnavailableSetting(SE_TRILINEARFILTER, hWnd, IDCHECKBOX_TRILINEARFILTER);
    DisableUnavailableSetting(SE_ISVOODOO, hWnd, IDCHECKBOX_VOODOO);
    DisableUnavailableSetting(SE_MOUSEEXCLUSIVE, hWnd, IDCHECKBOX_MOUSEEXCLUSIVE);
}

static void __stdcall SetSoundTab(HWND hWnd)
{
    // initialize static combo boxes
    UI::FillComboBoxMUI(hWnd, IDCOMBOBOX_SOUND_MODE,        l_lpuSoundModes,       __ARRAYSIZE(l_lpuSoundModes)      );
    UI::FillComboBoxMUI(hWnd, IDCOMBOBOX_SOUND_SPEAKERTYPE, l_lpuSpeakerTypes,     __ARRAYSIZE(l_lpuSpeakerTypes)    );
    UI::FillComboBoxMUI(hWnd, IDCOMBOBOX_SOUND_BITRATE,     l_lpuDigitalRateTypes, __ARRAYSIZE(l_lpuDigitalRateTypes));
    UI::FillComboBoxMUI(hWnd, IDCOMBOBOX_SOUND_BITDEPTH,    l_lpuDigitalBitsTypes, __ARRAYSIZE(l_lpuDigitalBitsTypes));
    UI::FillComboBoxMUI(hWnd, IDCOMBOBOX_SOUND_CHANNELS,    l_lpuNumSampleTypes,   __ARRAYSIZE(l_lpuNumSampleTypes)  );

    // initialize track bars
    SendMessage(GetDlgItem(hWnd, IDTRACKBAR_STREAMVOLUME), TBM_SETRANGE, (WPARAM)FALSE, (LPARAM)MAKELONG(0, 127));
    SendMessage(GetDlgItem(hWnd, IDTRACKBAR_SOUNDVOLUME),  TBM_SETRANGE, (WPARAM)FALSE, (LPARAM)MAKELONG(0, 127));

    // reflect settings
    UI::BATCHLIST BatchList[] =
    {
        // initialize check boxes
        { IDCHECKBOX_BGMISPAUSED,       BM_SETCHECK,  (WPARAM)Settings->Get(SE_BGMISPAUSED),     0                                     },
        { IDCHECKBOX_ISSOUNDON,         BM_SETCHECK,  (WPARAM)!Settings->Get(SE_ISSOUNDON),      0                                     },
        // initialize track bars
        { IDTRACKBAR_STREAMVOLUME,      TBM_SETPOS,   (WPARAM)TRUE,                             (LPARAM)Settings->Get(SE_STREAMVOLUME) },
        { IDTRACKBAR_SOUNDVOLUME,       TBM_SETPOS,   (WPARAM)TRUE,                             (LPARAM)Settings->Get(SE_SOUNDVOLUME)  },
        // initialize combo boxes
        { IDCOMBOBOX_SOUND_MODE,        CB_SETCURSEL, (WPARAM)Settings->Get(SE_SOUNDMODE),       0                                     },
        { IDCOMBOBOX_SOUND_SPEAKERTYPE, CB_SETCURSEL, (WPARAM)Settings->Get(SE_SPEAKERTYPE),     0                                     },
        { IDCOMBOBOX_SOUND_BITRATE,     CB_SETCURSEL, (WPARAM)Settings->Get(SE_DIGITALRATETYPE), 0                                     },
        { IDCOMBOBOX_SOUND_BITDEPTH,    CB_SETCURSEL, (WPARAM)Settings->Get(SE_DIGITALBITSTYPE), 0                                     },
        { IDCOMBOBOX_SOUND_CHANNELS,    CB_SETCURSEL, (WPARAM)Settings->Get(SE_NUMSAMPLETYPE),   0                                     },
    };
    UI::BatchMessage(hWnd, BatchList, __ARRAYSIZE(BatchList));

    // dependencies
    EnableWindow(GetDlgItem(hWnd, IDTRACKBAR_STREAMVOLUME), (BOOL)(IsDlgButtonChecked(hWnd, IDCHECKBOX_BGMISPAUSED)!=BST_CHECKED));
    EnableWindow(GetDlgItem(hWnd, IDTRACKBAR_SOUNDVOLUME), (BOOL)(IsDlgButtonChecked(hWnd, IDCHECKBOX_ISSOUNDON)!=BST_CHECKED));

    // store availability
    DisableUnavailableSetting(SE_SPEAKERTYPE, hWnd, IDCOMBOBOX_SOUND_SPEAKERTYPE);
    DisableUnavailableSetting(SE_DIGITALRATETYPE, hWnd, IDCOMBOBOX_SOUND_BITRATE);
    DisableUnavailableSetting(SE_DIGITALBITSTYPE, hWnd, IDCOMBOBOX_SOUND_BITDEPTH);
    DisableUnavailableSetting(SE_NUMSAMPLETYPE, hWnd, IDCOMBOBOX_SOUND_CHANNELS);
}

static void __stdcall SetSetupTab(HWND hWnd)
{
    // reflect settings
    UI::BATCHLIST BatchList[] =
    {
        // initialize check boxes
        { IDCHECKBOX_AURA,              BM_SETCHECK, (WPARAM)Settings->Get(SE_AURA),              0 },
        { IDCHECKBOX_ISFIXEDCAMERA,     BM_SETCHECK, (WPARAM)Settings->Get(SE_ISFIXEDCAMERA),     0 },
        { IDCHECKBOX_ISEFFECTON,        BM_SETCHECK, (WPARAM)Settings->Get(SE_ISEFFECTON),        0 },
        { IDCHECKBOX_ONHOUSERAI,        BM_SETCHECK, (WPARAM)Settings->Get(SE_ONHOUSERAI),        0 },
        { IDCHECKBOX_ISITEMSNAP,        BM_SETCHECK, (WPARAM)Settings->Get(SE_ISITEMSNAP),        0 },
        { IDCHECKBOX_LOGINOUT,          BM_SETCHECK, (WPARAM)Settings->Get(SE_LOGINOUT),          0 },
        { IDCHECKBOX_ONMERUSERAI,       BM_SETCHECK, (WPARAM)Settings->Get(SE_ONMERUSERAI),       0 },
        { IDCHECKBOX_MAKEMISSEFFECT,    BM_SETCHECK, (WPARAM)Settings->Get(SE_MAKEMISSEFFECT),    0 },
        { IDCHECKBOX_MONSTERHP,         BM_SETCHECK, (WPARAM)Settings->Get(SE_MONSTERHP),         0 },
        { IDCHECKBOX_NOCTRL,            BM_SETCHECK, (WPARAM)Settings->Get(SE_NOCTRL),            0 },
        { IDCHECKBOX_NOSHIFT,           BM_SETCHECK, (WPARAM)Settings->Get(SE_NOSHIFT),           0 },
        { IDCHECKBOX_NOTRADE,           BM_SETCHECK, (WPARAM)Settings->Get(SE_NOTRADE),           0 },
        { IDCHECKBOX_SHOPPING,          BM_SETCHECK, (WPARAM)Settings->Get(SE_SHOPPING),          0 },
        { IDCHECKBOX_SNAP,              BM_SETCHECK, (WPARAM)Settings->Get(SE_SNAP),              0 },
        { IDCHECKBOX_SKILLFAIL,         BM_SETCHECK, (WPARAM)Settings->Get(SE_SKILLFAIL),         0 },
        { IDCHECKBOX_NOTALKMSG,         BM_SETCHECK, (WPARAM)Settings->Get(SE_NOTALKMSG),         0 },
        { IDCHECKBOX_NOTALKMSG2,        BM_SETCHECK, (WPARAM)Settings->Get(SE_NOTALKMSG2),        0 },
        { IDCHECKBOX_Q1,                BM_SETCHECK, (WPARAM)Settings->Get(SE_Q1),                0 },
        { IDCHECKBOX_Q2,                BM_SETCHECK, (WPARAM)Settings->Get(SE_Q2),                0 },
        { IDCHECKBOX_SHOWNAME,          BM_SETCHECK, (WPARAM)Settings->Get(SE_SHOWNAME),          0 },
        { IDCHECKBOX_STATEINFO,         BM_SETCHECK, (WPARAM)Settings->Get(SE_STATEINFO),         0 },
        { IDCHECKBOX_SHOWTIPSATSTARTUP, BM_SETCHECK, (WPARAM)Settings->Get(SE_SHOWTIPSATSTARTUP), 0 },
        { IDCHECKBOX_WINDOW,            BM_SETCHECK, (WPARAM)Settings->Get(SE_WINDOW),            0 },
        { IDCHECKBOX_SKILLSNAP,         BM_SETCHECK, (WPARAM)Settings->Get(SE_SKILLSNAP),         0 },
        { IDCHECKBOX_RESETWINDOWS,      BM_SETCHECK, (WPARAM)0,                                   0 },
        { IDCHECKBOX_RESETSKILLLV,      BM_SETCHECK, (WPARAM)0,                                   0 },
        { IDCHECKBOX_RESETFOLDER,       BM_SETCHECK, (WPARAM)0,                                   0 },
        { IDCHECKBOX_RESETSETTING,      BM_SETCHECK, (WPARAM)0,                                   0 },
    };
    UI::BatchMessage(hWnd, BatchList, __ARRAYSIZE(BatchList));

    // dependencies
    EnableWindow(GetDlgItem(hWnd, IDCHECKBOX_RESETWINDOWS), TRUE);
    EnableWindow(GetDlgItem(hWnd, IDCHECKBOX_RESETSKILLLV), TRUE);
    //EnableWindow(GetDlgItem(hWnd, IDCHECKBOX_RESETFOLDER),  TRUE);

    // store availability
    DisableUnavailableSetting(SE_AURA, hWnd, IDCHECKBOX_AURA);
    DisableUnavailableSetting(SE_ISFIXEDCAMERA, hWnd, IDCHECKBOX_ISFIXEDCAMERA);
    DisableUnavailableSetting(SE_ISEFFECTON, hWnd, IDCHECKBOX_ISEFFECTON);
    DisableUnavailableSetting(SE_ONHOUSERAI, hWnd, IDCHECKBOX_ONHOUSERAI);
    DisableUnavailableSetting(SE_ISITEMSNAP, hWnd, IDCHECKBOX_ISITEMSNAP);
    DisableUnavailableSetting(SE_LOGINOUT, hWnd, IDCHECKBOX_LOGINOUT);
    DisableUnavailableSetting(SE_ONMERUSERAI, hWnd, IDCHECKBOX_ONMERUSERAI);
    DisableUnavailableSetting(SE_MAKEMISSEFFECT, hWnd, IDCHECKBOX_MAKEMISSEFFECT);
    DisableUnavailableSetting(SE_MONSTERHP, hWnd, IDCHECKBOX_MONSTERHP);
    DisableUnavailableSetting(SE_NOCTRL, hWnd, IDCHECKBOX_NOCTRL);
    DisableUnavailableSetting(SE_NOSHIFT, hWnd, IDCHECKBOX_NOSHIFT);
    DisableUnavailableSetting(SE_NOTRADE, hWnd, IDCHECKBOX_NOTRADE);
    DisableUnavailableSetting(SE_Q1, hWnd, IDCHECKBOX_Q1);
    DisableUnavailableSetting(SE_Q2, hWnd, IDCHECKBOX_Q2);
    DisableUnavailableSetting(SE_SHOPPING, hWnd, IDCHECKBOX_SHOPPING);
    DisableUnavailableSetting(SE_SNAP, hWnd, IDCHECKBOX_SNAP);
    DisableUnavailableSetting(SE_SKILLFAIL, hWnd, IDCHECKBOX_SKILLFAIL);
    DisableUnavailableSetting(SE_NOTALKMSG, hWnd, IDCHECKBOX_NOTALKMSG);
    DisableUnavailableSetting(SE_NOTALKMSG2, hWnd, IDCHECKBOX_NOTALKMSG2);
    DisableUnavailableSetting(SE_SHOWNAME, hWnd, IDCHECKBOX_SHOWNAME);
    DisableUnavailableSetting(SE_STATEINFO, hWnd, IDCHECKBOX_STATEINFO);
    DisableUnavailableSetting(SE_SHOWTIPSATSTARTUP, hWnd, IDCHECKBOX_SHOWTIPSATSTARTUP);
    DisableUnavailableSetting(SE_WINDOW, hWnd, IDCHECKBOX_WINDOW);
    DisableUnavailableSetting(SE_SKILLSNAP, hWnd, IDCHECKBOX_SKILLSNAP);
}

static BOOL CALLBACK VideoTabProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    unsigned long luIdx;

    switch(uMsg)
    {
        case WM_INITDIALOG:
        {
            bool bActive = false;

            if(UI::IsRemoteSession())
            {
                g_Log.LogWarning("Remote desktop session detected.");
            }
            else
            {
                ShowWindow(GetDlgItem(hWnd, IDINFOICON_VIDEO_REMOTESESSION), SW_HIDE);
            }

            if(UI::IsMirrorDriverPresent(&bActive))
            {
                if(bActive)
                {
                    g_Log.LogWarning("Mirror video driver (attached) detected.");
                }
                else
                {
                    g_Log.LogWarning("Mirror video driver detected.");
                }
            }
            if(!bActive)
            {
                ShowWindow(GetDlgItem(hWnd, IDINFOICON_VIDEO_MIRRORDRIVER), SW_HIDE);
            }

            return FALSE;
        }
        case WM_COMMAND:
            switch(HIWORD(wParam))
            {
                case BN_CLICKED:
                    switch(LOWORD(wParam))
                    {
                        case IDCHECKBOX_HELDEVICE:
                            OnUpdateDevice(hWnd);
                            break;
                        default:
                            return FALSE;
                    }
                    break;
                case CBN_SELCHANGE:
                    switch(LOWORD(wParam))
                    {
                        case IDCOMBOBOX_VIDEODEVICE:
                            if((luIdx = SendMessage((HWND)lParam, CB_GETCURSEL, 0, 0))!=CB_ERR)
                            {// required for update resolution
                                Settings->Set(SE_DEVICECNT, luIdx);
                            }
                            OnChangeDevice(hWnd);
                            break;
                        default:
                            return FALSE;
                    }
                    break;
                default:
                    return FALSE;
            }
            break;
        case WM_NOTIFY:
            UI::HandleToolTips((LPNMHDR)lParam);
            break;
        default:
            return FALSE;
    }
    return TRUE;
}

static BOOL CALLBACK SoundTabProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch(uMsg)
    {
        case WM_INITDIALOG:
        {
            bool bExist = false;

            if(!Sound::IsMSSSane(&bExist))
            {
                if(bExist)
                {
                    g_Log.LogWarning("Corrupt MSS driver files.");
                }
                else
                {
                    g_Log.LogWarning("Missing MSS driver files.");
                }
            }
            else
            {
                ShowWindow(GetDlgItem(hWnd, IDINFOICON_SOUND_MISSINGFILES), SW_HIDE);
            }

            return FALSE;
        }
        case WM_COMMAND:
            switch(HIWORD(wParam))
            {
                case BN_CLICKED:
                    switch(LOWORD(wParam))
                    {
                        case IDCHECKBOX_BGMISPAUSED:
                            EnableWindow(GetDlgItem(hWnd, IDTRACKBAR_STREAMVOLUME), (BOOL)(IsDlgButtonChecked(hWnd, IDCHECKBOX_BGMISPAUSED)!=BST_CHECKED));
                            break;
                        case IDCHECKBOX_ISSOUNDON:
                            EnableWindow(GetDlgItem(hWnd, IDTRACKBAR_SOUNDVOLUME), (BOOL)(IsDlgButtonChecked(hWnd, IDCHECKBOX_ISSOUNDON)!=BST_CHECKED));
                            break;
                        default:
                            return FALSE;
                    }
                    break;
                default:
                    return FALSE;
            }
            break;
        case WM_NOTIFY:
            UI::HandleToolTips((LPNMHDR)lParam);
            break;
        default:
            return FALSE;
    }
    return TRUE;
}

static BOOL CALLBACK SetupTabProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch(uMsg)
    {
        case WM_INITDIALOG:
            return FALSE;
        case WM_COMMAND:
            switch(HIWORD(wParam))
            {
                case BN_CLICKED:
                    switch(LOWORD(wParam))
                    {
                        case IDCHECKBOX_RESETSETTING:
                        {
                            BOOL bChecked = (BOOL)(IsDlgButtonChecked(hWnd, IDCHECKBOX_RESETSETTING)==BST_CHECKED);

                            // (un)check all
                            UI::BATCHLIST BatchList[] =
                            {
                                { IDCHECKBOX_RESETWINDOWS, BM_SETCHECK, (WPARAM)bChecked, 0 },
                                { IDCHECKBOX_RESETSKILLLV, BM_SETCHECK, (WPARAM)bChecked, 0 },
                                //{ IDCHECKBOX_RESETFOLDER,  BM_SETCHECK, (WPARAM)bChecked, 0 },
                            };
                            UI::BatchMessage(hWnd, BatchList, __ARRAYSIZE(BatchList));

                            // dis/enable all
                            EnableWindow(GetDlgItem(hWnd, IDCHECKBOX_RESETWINDOWS), !bChecked);
                            EnableWindow(GetDlgItem(hWnd, IDCHECKBOX_RESETSKILLLV), !bChecked);
                            //EnableWindow(GetDlgItem(hWnd, IDCHECKBOX_RESETFOLDER),  !bChecked);
                            break;
                        }
                        default:
                            return FALSE;
                    }
                    break;
                default:
                    return FALSE;
            }
            break;
        case WM_NOTIFY:
            UI::HandleToolTips((LPNMHDR)lParam);
            break;
        default:
            return FALSE;
    }
    return TRUE;
}

static BOOL CALLBACK AboutTabProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch(uMsg)
    {
        case WM_INITDIALOG:
            {
                SetWindowText(GetDlgItem(hWnd, IDC_ABOUT_TITLE), APP_TITLE);
                SetWindowText(GetDlgItem(hWnd, IDC_ABOUT_CORPSE), APP_CORPSE);
            }
            return FALSE;
        case WM_CTLCOLORSTATIC:
            if(GetDlgItem(hWnd, IDC_ABOUT_CORPSE)!=(HWND)lParam)
            {// not the EDIT box
                return FALSE;
            }

            SetTextColor((HDC)wParam, GetSysColor(COLOR_WINDOWTEXT));

            // make readonly/disabled edit box have white background
            return (BOOL)GetSysColorBrush(COLOR_WINDOW);
        case WM_COMMAND:
            switch(HIWORD(wParam))
            {
                case BN_CLICKED:
                    switch(LOWORD(wParam))
                    {
                        case IDHOMEPAGE:
                        {
                            SHELLEXECUTEINFO Sei = { sizeof(Sei) };

                            Sei.hwnd = hWnd;
                            Sei.lpVerb = NULL;
                            Sei.lpFile = "http://ai4rei.net/p/opensetup";
                            Sei.nShow = SW_SHOWNORMAL;

                            EnableWindow(GetDlgItem(hWnd, IDHOMEPAGE), FALSE);
                            ShellExecuteEx(&Sei);
                            EnableWindow(GetDlgItem(hWnd, IDHOMEPAGE), TRUE);
                            break;
                        }
                        default:
                            return FALSE;
                    }
                    break;
                default:
                    return FALSE;
            }
            break;
        default:
            return FALSE;
    }
    return TRUE;
}

static void __stdcall OnChangeSettingsEngine(HWND hWnd)
{
    HICON hOldIcon = NULL;
    HINSTANCE hInstance = GetModuleHandle(NULL);
    SETTINGENGINEID nEngineID = Settings->GetEngineID();

    g_Log.LogInfo("OnChangeSettingsEngine: %s", &nEngineID);

    // disable tool tips
    UI::EnableToolTips(hWnd, FALSE);

    // set icon
    switch(nEngineID)
    {
        case SENGINE_LUA:
            hOldIcon = (HICON)SendMessage(GetDlgItem(hWnd, IDELOGO), STM_SETIMAGE, IMAGE_ICON,
                (LPARAM)LoadImage(hInstance, MAKEINTRESOURCE(IDI_ENGINE_LUA), IMAGE_ICON, 0, 0, 0));
            break;
        case SENGINE_REG:
            hOldIcon = (HICON)SendMessage(GetDlgItem(hWnd, IDELOGO), STM_SETIMAGE, IMAGE_ICON,
                (LPARAM)LoadImage(hInstance, MAKEINTRESOURCE(IDI_ENGINE_REG), IMAGE_ICON, 0, 0, 0));
            break;
    }
    if(hOldIcon)
    {
        DestroyIcon(hOldIcon);
        hOldIcon = NULL;
    }

    // recall storage
    Settings->Load();

    // initialize tabs
    SetVideoTab(TabMgr.GetTab(IDVIDEO));
    SetSoundTab(TabMgr.GetTab(IDSOUND));
    SetSetupTab(TabMgr.GetTab(IDSETUP));

    // setup UAC shield as necessary
    bool bAdmin = Settings->IsAdminRequired();
    UI::SetButtonShield(GetDlgItem(hWnd, IDOK), bAdmin);
    UI::SetButtonShield(GetDlgItem(hWnd, IDAPPLY), bAdmin);

    // setup tool tips
    UI::EnableToolTips(hWnd, TRUE);
}

static bool __stdcall OnBackgroundSave(HWND hWnd, unsigned long luHash)
{
    char szSlave[MAX_PATH], szParam[128];
    DWORD dwExitCode = EXIT_FAILURE;
    SHELLEXECUTEINFO Sei = { sizeof(Sei), SEE_MASK_FLAG_NO_UI|SEE_MASK_NOCLOSEPROCESS, hWnd, "runas", szSlave, szParam, ".", SW_SHOWDEFAULT };

    GetModuleFileNameA(NULL, szSlave, __ARRAYSIZE(szSlave));
    wsprintfA(szParam, "/save:%lu,%lu", Settings->GetEngineID(), luHash);

    // disable interaction
    EnableWindow(hWnd, FALSE);

    if(ShellExecuteEx(&Sei) && Sei.hProcess)
    {
        while(MsgWaitForMultipleObjects(1, &Sei.hProcess, FALSE, INFINITE, QS_ALLINPUT)!=WAIT_OBJECT_0)
        {
            MSG Msg;

            while(PeekMessage(&Msg, NULL, 0, 0, PM_REMOVE))
            {
                TranslateMessage(&Msg);
                DispatchMessage(&Msg);
            }
        }
        GetExitCodeProcess(Sei.hProcess, &dwExitCode);
        CloseHandle(Sei.hProcess);
    }
    else if(GetLastError()!=ERROR_CANCELLED)  // do not report UAC prompt cancelation as per UAC guidelines
    {
        CError::ErrorMessage(hWnd, TEXT_ERROR_IPC_FAILED);
    }

    // re-enable interaction
    EnableWindow(hWnd, TRUE);

    return (dwExitCode==EXIT_SUCCESS) ? true : false;
}

static BOOL CALLBACK MainDialogProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    HINSTANCE hInstance = GetModuleHandle(NULL);

    switch(uMsg)
    {
        case WM_INITDIALOG:
        {
            // give the dialog window icons
            SendMessage(hWnd, WM_SETICON, ICON_BIG,
                (LPARAM)LoadImage(hInstance, MAKEINTRESOURCE(IDI_MAINICON), IMAGE_ICON, GetSystemMetrics(SM_CXICON), GetSystemMetrics(SM_CYICON), 0));
            SendMessage(hWnd, WM_SETICON, ICON_SMALL,
                (LPARAM)LoadImage(hInstance, MAKEINTRESOURCE(IDI_MAINICON), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0));

            // initialize tab manager
            if(TabMgr.Init(hInstance, hWnd, 16, IDB_IMAGELIST_MASK, IMI_MAX))
            {
                char szTabTitle[32];

                // graphics
                LoadStringA(hInstance, TEXT_TAB_GRAPHICS, szTabTitle, __ARRAYSIZE(szTabTitle));
                TabMgr.AddTab(szTabTitle, IDVIDEO, IMI_VIDEO, true, MAKEINTRESOURCE(IDD_TAB_VIDEO), &VideoTabProc);

                // sound
                LoadStringA(hInstance, TEXT_TAB_SOUNDS, szTabTitle, __ARRAYSIZE(szTabTitle));
                TabMgr.AddTab(szTabTitle, IDSOUND, IMI_SOUND, false, MAKEINTRESOURCE(IDD_TAB_SOUND), &SoundTabProc);

                // general
                LoadStringA(hInstance, TEXT_TAB_SETTINGS, szTabTitle, __ARRAYSIZE(szTabTitle));
                TabMgr.AddTab(szTabTitle, IDSETUP, IMI_SETUP, false, MAKEINTRESOURCE(IDD_TAB_SETUP), &SetupTabProc);

                // add-ons
                ROExt.Load(&TabMgr);

                // about
                LoadStringA(hInstance, TEXT_TAB_ABOUT, szTabTitle, __ARRAYSIZE(szTabTitle));
                TabMgr.AddTab(szTabTitle, IDABOUT, IMI_ABOUT, false, MAKEINTRESOURCE(IDD_TAB_ABOUT), &AboutTabProc);
            }

            // load MUI
            UI::LoadMUI(hWnd);

            // load settings and refresh tabs
            OnChangeSettingsEngine(hWnd);

            // get shell font (to satisfy multibyte charsets)
            if(!l_DlgFont)
            {
                l_DlgFont = UI::GetShellFont();
            }

            UI::SetFont(hWnd, l_DlgFont);

            return FALSE;
        }
        case WM_CLOSE:
            DestroyWindow(hWnd);
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        case WM_COMMAND:
            switch(HIWORD(wParam))
            {
                case 0:
                {
                    bool bSuccess = false;

                    switch(LOWORD(wParam))
                    {
                        case IDOK:
                        case IDAPPLY:
                            // disable whatever was pressed (visual cue)
                            EnableWindow(GetDlgItem(hWnd, LOWORD(wParam)), FALSE);

                            // save add-ons (TODO: UAC?)
                            ROExt.Save();

                            // collect data from tabs
                            GetVideoTab(TabMgr.GetTab(IDVIDEO));
                            GetSoundTab(TabMgr.GetTab(IDSOUND));
                            GetSetupTab(TabMgr.GetTab(IDSETUP));

                            // are we sane?
                            if(!Settings->IsSane())
                            {
                                g_Log.Store(true);
                                CError::ErrorMessage(hWnd, TEXT_ERROR_INSANE_ENGINE);
                            }

                            // persist storage
                            if(Settings->IsAdminRequired())
                            {
                                bSuccess = OnBackgroundSave(hWnd, Settings->SaveToIPC());
                            }
                            else
                            {
                                bSuccess = Settings->Save();
                            }

                            // re-enable button
                            EnableWindow(GetDlgItem(hWnd, LOWORD(wParam)), TRUE);

                            if(!bSuccess && !l_LoadDefaults)
                            {// abort on failure, unless we are loading defaults
                                break;
                            }

                            if(LOWORD(wParam)==IDAPPLY)
                            {// do not quit when 'Apply'
                                OnChangeSettingsEngine(hWnd);  // reflect
                                break;
                            }
                            // but fall through for 'OK'
                        case IDCANCEL:
                            if(LOWORD(wParam)==IDCANCEL && ParentCtrl.IsAvail() && (GetAsyncKeyState(VK_CONTROL)>>0xf))
                            {// get rid of the parent process that might intent to launch us again
                                ParentCtrl.Kill();
                            }

                            DestroyWindow(hWnd);
                            //EndDialog(hWnd, LOWORD(wParam));
                            break;
                        case IDVIDEO:
                        case IDSOUND:
                        case IDSETUP:
                        case IDROEXT:
                        case IDABOUT:
                            TabMgr.OnActivateTab(LOWORD(wParam));
                            break;
                        case IDELOGO:
                            switch(Settings->GetEngineID())
                            {
                                case SENGINE_LUA:
                                    Settings = &SettingsReg;
                                    break;
                                case SENGINE_REG:
                                    Settings = &SettingsLua;
                                    break;
                            }

                            // load settings and refresh tabs
                            OnChangeSettingsEngine(hWnd);
                            break;
                        default:
                            return FALSE;
                    }
                    break;
                }
                case 1:
                    switch(LOWORD(wParam))
                    {
                        case IDC_TABNEXT:
                            TabMgr.ActivateTabNext();
                            break;
                        case IDC_TABPREV:
                            TabMgr.ActivateTabPrev();
                            break;
                        default:
                            return FALSE;
                    }
                    break;
                default:
                    return FALSE;
            }
            break;
        default:
            return FALSE;
    }
    return TRUE;
}

static int __cdecl DX7E_P_GetInfoSort(const void* lpOne, const void* lpTwo)
{
    LPCDX7EDISPLAYMODE const lpModeA = static_cast< LPCDX7EDISPLAYMODE >(lpOne);
    LPCDX7EDISPLAYMODE const lpModeB = static_cast< LPCDX7EDISPLAYMODE >(lpTwo);

    if(lpModeA->luWidth==lpModeB->luWidth)
    {
        return lpModeA->luHeight-lpModeB->luHeight;
    }

    return lpModeA->luWidth-lpModeB->luWidth;
}

static bool __stdcall DX7E_P_GetInfo(void)
{
    unsigned long luDrivers, luModes, luDevices, luBitDepthFlt = 16;
    OSVERSIONINFO Osvi = { sizeof(Osvi) };

    // retrieve direct x information
    if(!DX7E_EnumDriverDevices(&l_DI))
    {
        return false;
    }

    // windows 8 does not support 16 bit colored modes, so stick
    // with 32-bit
    if(GetVersionExA(&Osvi))
    {
        if(WIN32_VER_CHECK(&Osvi, VER_PLATFORM_WIN32_NT, 6, 2))
        {
            luBitDepthFlt = 32;
        }
    }

    g_Log.LogInfo("Enumerated %lu DX7 drivers.", l_DI.luDrivers);
    g_Log.IncrementLevel();

    // sort out irrelevant stuff
    for(luDrivers=0; luDrivers<l_DI.luDrivers; luDrivers++)
    {
        DX7EDISPLAYDRIVER* const lpDriver = &l_DI.Driver[luDrivers];
        unsigned long luPrevModes = lpDriver->luModes;

        for(luModes = lpDriver->luModes; luModes>0; luModes--)
        {
            const DX7EDISPLAYMODE* const lpMode = &lpDriver->Mode[luModes-1];

            if(lpMode->luWidth<640 || lpMode->luHeight<480 || lpMode->luBitDepth!=luBitDepthFlt)
            {// setup imposed limitations
                if(luModes<lpDriver->luModes)
                {
                    MoveMemory(&lpDriver->Mode[luModes-1], &lpDriver->Mode[luModes], (lpDriver->luModes-luModes)*sizeof(lpDriver->Mode[0]));
                }

                lpDriver->luModes--;
            }
        }

        if(lpDriver->luModes)
        {// setup sorts modes
            qsort(lpDriver->Mode, lpDriver->luModes, sizeof(l_DI.Driver[0].Mode[0]), &DX7E_P_GetInfoSort);
        }

        g_Log.LogInfo("Driver '%s': %lu video modes (%lu filtered), %lu hw devices, %lu sw devices", lpDriver->szName, luPrevModes, luPrevModes-lpDriver->luModes, lpDriver->luDevicesHW, lpDriver->luDevicesSW);

        g_Log.IncrementLevel();

        for(luDevices = 0; luDevices<lpDriver->luDevicesHW; luDevices++)
        {
            g_Log.LogInfo("HW Device '%s': Type %d", lpDriver->DeviceHW[luDevices].szName, lpDriver->DeviceHW[luDevices].nType);
        }

        for(luDevices = 0; luDevices<lpDriver->luDevicesSW; luDevices++)
        {
            g_Log.LogInfo("SW Device '%s': Type %d", lpDriver->DeviceSW[luDevices].szName, lpDriver->DeviceSW[luDevices].nType);
        }

        g_Log.DecrementLevel();
    }

    g_Log.DecrementLevel();

    return true;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, char* lpCmdLine, int nShowCmd)
{
    // select registry target
    if(g_Config.GetBool("settings", "HKLMtoHKCU", false))
    {
        HKEY_GRAVITY = HKEY_CURRENT_USER;
    }

    // handle parameters
    if(lpCmdLine[0]=='/')
    {
        unsigned long luEngine = 0, luHash = 0;

        if(sscanf(lpCmdLine, "/save:%lu,%lu", &luEngine, &luHash)==2)
        {// handle ipc save request
            switch(luEngine)
            {
                case SENGINE_LUA:
                    Settings = &SettingsLua;
                    break;
                case SENGINE_REG:
                    Settings = &SettingsReg;
                    break;
                default:
                    return EXIT_FAILURE;
            }

            Settings->LoadFromIPC(luHash);
            return Settings->Save() ? EXIT_SUCCESS : EXIT_FAILURE;
        }
        else if(!strcmp(lpCmdLine, "/defaults"))
        {// handle defaults setup
            l_LoadDefaults = TRUE;
        }
    }

    // Windows 8 PCA cleanup.
    for(;;)
    {
        CHAR szFileName[MAX_PATH];
        HKEY hKey;
        LONG lResult;
        OSVERSIONINFO Osvi = { sizeof(Osvi) };

        if(!GetVersionExA(&Osvi))
        {// should not happen
            break;
        }

        if(!WIN32_VER_CHECK(&Osvi, VER_PLATFORM_WIN32_NT, 6, 2))
        {// it's a "feature"
            break;
        }

        GetModuleFileNameA(NULL, szFileName, __ARRAYSIZE(szFileName));

        lResult = RegOpenKeyExA(
            HKEY_CURRENT_USER,                                                          // handle this per user
            "Software\\Microsoft\\Windows NT\\CurrentVersion\\AppCompatFlags\\Layers",  // PCA compatibility flags
            0,                                                                          // reserved
            KEY_READ|KEY_WRITE,                                                         // read & write
            &hKey                                                                       // destination key handle
        );

        if(lResult!=ERROR_SUCCESS)
        {// not found, good
            break;
        }

        // delete our entry unconditionally
        RegDeleteValue(hKey, szFileName);
        RegCloseKey(hKey);
        break;
    }

    HANDLE hMutex = CreateMutex(NULL, FALSE, "Global\\OpenSetupSIMutex");

    // prevent multiple instances from running
    if(!hMutex)
    {
        CError::ErrorMessage(NULL, TEXT_ERROR_INIT_MUTEX);
        return EXIT_FAILURE;
    }
    else if(WaitForSingleObject(hMutex, 0)==WAIT_TIMEOUT)
    {
        CloseHandle(hMutex);
        return EXIT_SUCCESS;
    }

    INITCOMMONCONTROLSEX Icce = { sizeof(Icce), ICC_WIN95_CLASSES };

    // init common controls
    if(!InitCommonControlsEx(&Icce))
    {
        CError::ErrorMessage(NULL, TEXT_ERROR_INIT_COMCTL32);
        return EXIT_FAILURE;
    }

    // init direct x information
    if(!DX7E_P_GetInfo())
    {
        CError::ErrorMessage(NULL, TEXT_ERROR_INIT_DIRECTX7);
    }

    unsigned long luEngine = SENGINE_LUA;

    // we are reading strings into unsigned longs, aren't we great?
    // check settings.h if you are still in doubt.
    g_Config.GetString("settings", "Engine", (char*)&luEngine, (char*)&luEngine, sizeof(luEngine));

    // select settings engine
    switch(luEngine)
    {
        case SENGINE_LUA:
            Settings = &SettingsLua;
            break;
        case SENGINE_REG:
            Settings = &SettingsReg;
            break;
        default:
            CError::ErrorMessage(NULL, TEXT_ERROR_UNKNOWN_ENGINE, luEngine);
            return EXIT_FAILURE;
    }

    // show 'em the dialog
    HACCEL hAccel = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDA_MAIN_DIALOG));
    HWND hWnd = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_MAIN_DIALOG), NULL, &MainDialogProc);

    // if we are only loading the machine with defaults, tear it down
    if(l_LoadDefaults)
    {
        PostMessage(hWnd, WM_COMMAND, MAKELONG(IDOK, 0), (LPARAM)GetDlgItem(hWnd, IDOK));
    }
    else
    {// otherwise make it show according start up options
        ShowWindow(hWnd, nShowCmd);
    }

    // process messages
    MSG Msg;

    while(GetMessage(&Msg, NULL, 0, 0)>0)
    {
        if(!TranslateAccelerator(hWnd, hAccel, &Msg))
        {
            if(!IsDialogMessage(hWnd, &Msg))
            {
                TranslateMessage(&Msg);
                DispatchMessage(&Msg);
            }
        }
    }

    // free font, if any
    if(l_DlgFont)
    {
        DeleteObject(l_DlgFont);
        l_DlgFont = NULL;
    }

    // release mutex
    CloseHandle(hMutex);

    // dump log to disk
    if(GetAsyncKeyState(VK_SHIFT)>>0xf)
    {
        g_Log.Store(true);
    }

    return EXIT_SUCCESS;
}
