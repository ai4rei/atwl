// -----------------------------------------------------------------
// RagnarokOnline OpenSetup
// (c) 2010-2013 Ai4rei/AN
// See doc/license.txt for details.
//
// -----------------------------------------------------------------

#define DIRECT3D_VERSION 0x0700
#define DIRECTDRAW_VERSION 0x0700
#include <windows.h>
#include <commctrl.h>
#include <d3d.h>
#include <ddraw.h>

#include <regutil.h>

#include "dx7enum.h"
#include "error.h"
#include "resource.h"
#include "roext.h"
#include "settings.h"
#include "settings_lua.h"
#include "settings_reg.h"
#include "tab.h"
#include "ui.h"

#include "opensetup.h"

static const char* l_lpszSignature = "$ RagnarokOnline OpenSetup, "APP_VERSION", build on "__DATE__" @ "__TIME__", (c) 2010-2013 Ai4rei/AN $";

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

static HFONT l_DlgFont = NULL;
static struct DDrawDriverDeviceInfo l_DI = { 0 };
static class CROExt ROExt;
static class CTabMgr TabMgr;

static class CSettingsLua SettingsLua;
static class CSettingsReg SettingsReg;
static class CSettings* Settings = &SettingsLua;

static inline unsigned long __stdcall GetDriverIndex(unsigned long luComboIndex)
{
    unsigned long luDriver, luDevice;

    for(luDriver = 0; luDriver<l_DI.luItems; luDriver++)
    {
        for(luDevice = 0; luDevice<l_DI.Drivers[luDriver].luItems; luDevice++)
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

    for(luDriver = 0; luDriver<l_DI.luItems; luDriver++)
    {
        for(luDevice = 0; luDevice<l_DI.Drivers[luDriver].luItems; luDevice++)
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
    unsigned long i, lResult, luIndex;
    struct DDrawDriverEntry* lpEntry;
    HWND hResWnd;

    lResult = Settings->Get(SE_DEVICECNT);
    luIndex = GetDriverIndex(lResult);

    if(luIndex>=l_DI.luItems)
    {// ignore
        return;
    }

    lpEntry = &l_DI.Drivers[luIndex];

    // save selection
    hResWnd = GetDlgItem(hWnd, IDCOMBOBOX_RESOLUTION);
    lResult = SendMessage(hResWnd, CB_GETCURSEL, 0, 0);

    // empty combobox
    SendMessage(hResWnd, CB_RESETCONTENT, 0, 0);

    for(i=0; i<lpEntry->luModes; i++)
    {
        wsprintfA(szLabel, "%lu x %lu x %lu", lpEntry->Modes[i].luWidth, lpEntry->Modes[i].luHeight, lpEntry->Modes[i].luBitDepth);

        SendMessage(hResWnd, CB_ADDSTRING, 0, (LPARAM)szLabel);
    }

    // restore selection
    SendMessage(hResWnd, CB_SETCURSEL, (WPARAM)lResult, 0);
}

static void __stdcall GetVideoTab(HWND hWnd)
{
    unsigned long i;

    // complex combo boxes
    if((i = SendMessage(GetDlgItem(hWnd, IDCOMBOBOX_VIDEODEVICE), CB_GETCURSEL, 0, 0))!=CB_ERR)
    {
        unsigned long luIndex = GetDriverIndex(i);

        Settings->Set(SE_DEVICECNT, i);
        Settings->Set(SE_GUIDDRIVER, &l_DI.Drivers[luIndex].DriverGuid);
        Settings->Set(SE_GUIDDEVICE, &l_DI.Drivers[luIndex].Devices[GetDeviceIndex(i)].DeviceGuid);
        Settings->Set(SE_DEVICENAME,  l_DI.Drivers[luIndex].Devices[GetDeviceIndex(i)].szName);
    }
    if((i = SendMessage(GetDlgItem(hWnd, IDCOMBOBOX_RESOLUTION), CB_GETCURSEL, 0, 0))!=CB_ERR)
    {
        unsigned long luIndex = GetDriverIndex(Settings->Get(SE_DEVICECNT));

        Settings->Set(SE_MODECNT,     i);
        Settings->Set(SE_WIDTH,       l_DI.Drivers[luIndex].Modes[i].luWidth   );
        Settings->Set(SE_HEIGHT,      l_DI.Drivers[luIndex].Modes[i].luHeight  );
        Settings->Set(SE_BITPERPIXEL, l_DI.Drivers[luIndex].Modes[i].luBitDepth);
    }

    // check boxes
    Settings->Set(SE_ISFULLSCREENMODE, SendMessage(GetDlgItem(hWnd, IDCHECKBOX_FULLSCREEN),     BM_GETCHECK, 0, 0));
    Settings->Set(SE_FOG,              SendMessage(GetDlgItem(hWnd, IDCHECKBOX_FOG),            BM_GETCHECK, 0, 0));
    Settings->Set(SE_ISLIGHTMAP,       SendMessage(GetDlgItem(hWnd, IDCHECKBOX_LIGHTMAP),       BM_GETCHECK, 0, 0));
    Settings->Set(SE_TRILINEARFILTER,  SendMessage(GetDlgItem(hWnd, IDCHECKBOX_TRILINEARFILTER),BM_GETCHECK, 0, 0));
    Settings->Set(SE_ISVOODOO,         SendMessage(GetDlgItem(hWnd, IDCHECKBOX_VOODOO),         BM_GETCHECK, 0, 0));
    Settings->Set(SE_MOUSEEXCLUSIVE,   SendMessage(GetDlgItem(hWnd, IDCHECKBOX_MOUSEEXCLUSIVE), BM_GETCHECK, 0, 0));

    // track bars
    Settings->Set(SE_SPRITEMODE,       SendMessage(GetDlgItem(hWnd, IDTRACKBAR_SPRITEQ),    TBM_GETPOS,  0, 0));
    Settings->Set(SE_TEXTUREMODE,      SendMessage(GetDlgItem(hWnd, IDTRACKBAR_TEXTUREQ),   TBM_GETPOS,  0, 0));
}

static void __stdcall GetSoundTab(HWND hWnd)
{
    unsigned long i;

    // simple combo boxes
    if((i = SendMessage(GetDlgItem(hWnd, IDCOMBOBOX_SOUND_MODE), CB_GETCURSEL, 0, 0))!=CB_ERR)
    {
        Settings->Set(SE_SOUNDMODE, i);
    }
    if((i = SendMessage(GetDlgItem(hWnd, IDCOMBOBOX_SOUND_SPEAKERTYPE), CB_GETCURSEL, 0, 0))!=CB_ERR)
    {
        Settings->Set(SE_SPEAKERTYPE, i);
    }
    if((i = SendMessage(GetDlgItem(hWnd, IDCOMBOBOX_SOUND_BITRATE), CB_GETCURSEL, 0, 0))!=CB_ERR)
    {
        Settings->Set(SE_DIGITALRATETYPE, i);
    }
    if((i = SendMessage(GetDlgItem(hWnd, IDCOMBOBOX_SOUND_BITDEPTH), CB_GETCURSEL, 0, 0))!=CB_ERR)
    {
        Settings->Set(SE_DIGITALBITSTYPE, i);
    }
    if((i = SendMessage(GetDlgItem(hWnd, IDCOMBOBOX_SOUND_CHANNELS), CB_GETCURSEL, 0, 0))!=CB_ERR)
    {
        Settings->Set(SE_NUMSAMPLETYPE, i);
    }

    // check boxes
    Settings->Set(SE_BGMISPAUSED, SendMessage(GetDlgItem(hWnd, IDCHECKBOX_BGMISPAUSED), BM_GETCHECK, 0, 0));
    Settings->Set(SE_ISSOUNDON,   !SendMessage(GetDlgItem(hWnd, IDCHECKBOX_ISSOUNDON),  BM_GETCHECK, 0, 0));

    // track bars
    Settings->Set(SE_STREAMVOLUME, SendMessage(GetDlgItem(hWnd, IDTRACKBAR_STREAMVOLUME), TBM_GETPOS,  0, 0));
    Settings->Set(SE_SOUNDVOLUME,  SendMessage(GetDlgItem(hWnd, IDTRACKBAR_SOUNDVOLUME),  TBM_GETPOS,  0, 0));
}

static void __stdcall GetSetupTab(HWND hWnd)
{
    // check boxes
    Settings->Set(SE_AURA,              SendMessage(GetDlgItem(hWnd, IDCHECKBOX_AURA),              BM_GETCHECK, 0, 0));
    Settings->Set(SE_ISFIXEDCAMERA,     SendMessage(GetDlgItem(hWnd, IDCHECKBOX_ISFIXEDCAMERA),     BM_GETCHECK, 0, 0));
    Settings->Set(SE_ISEFFECTON,        SendMessage(GetDlgItem(hWnd, IDCHECKBOX_ISEFFECTON),        BM_GETCHECK, 0, 0));
    Settings->Set(SE_ONHOUSERAI,        SendMessage(GetDlgItem(hWnd, IDCHECKBOX_ONHOUSERAI),        BM_GETCHECK, 0, 0));
    Settings->Set(SE_ISITEMSNAP,        SendMessage(GetDlgItem(hWnd, IDCHECKBOX_ISITEMSNAP),        BM_GETCHECK, 0, 0));
    Settings->Set(SE_LOGINOUT,          SendMessage(GetDlgItem(hWnd, IDCHECKBOX_LOGINOUT),          BM_GETCHECK, 0, 0));
    Settings->Set(SE_ONMERUSERAI,       SendMessage(GetDlgItem(hWnd, IDCHECKBOX_ONMERUSERAI),       BM_GETCHECK, 0, 0));
    Settings->Set(SE_MAKEMISSEFFECT,    SendMessage(GetDlgItem(hWnd, IDCHECKBOX_MAKEMISSEFFECT),    BM_GETCHECK, 0, 0));
    Settings->Set(SE_NOCTRL,            SendMessage(GetDlgItem(hWnd, IDCHECKBOX_NOCTRL),            BM_GETCHECK, 0, 0));
    Settings->Set(SE_NOSHIFT,           SendMessage(GetDlgItem(hWnd, IDCHECKBOX_NOSHIFT),           BM_GETCHECK, 0, 0));
    Settings->Set(SE_NOTRADE,           SendMessage(GetDlgItem(hWnd, IDCHECKBOX_NOTRADE),           BM_GETCHECK, 0, 0));
    Settings->Set(SE_SHOPPING,          SendMessage(GetDlgItem(hWnd, IDCHECKBOX_SHOPPING),          BM_GETCHECK, 0, 0));
    Settings->Set(SE_SNAP,              SendMessage(GetDlgItem(hWnd, IDCHECKBOX_SNAP),              BM_GETCHECK, 0, 0));
    Settings->Set(SE_SKILLFAIL,         SendMessage(GetDlgItem(hWnd, IDCHECKBOX_SKILLFAIL),         BM_GETCHECK, 0, 0));
    Settings->Set(SE_NOTALKMSG,         SendMessage(GetDlgItem(hWnd, IDCHECKBOX_NOTALKMSG),         BM_GETCHECK, 0, 0));
    Settings->Set(SE_NOTALKMSG2,        SendMessage(GetDlgItem(hWnd, IDCHECKBOX_NOTALKMSG2),        BM_GETCHECK, 0, 0));
    Settings->Set(SE_SHOWNAME,          SendMessage(GetDlgItem(hWnd, IDCHECKBOX_SHOWNAME),          BM_GETCHECK, 0, 0));
    Settings->Set(SE_STATEINFO,         SendMessage(GetDlgItem(hWnd, IDCHECKBOX_STATEINFO),         BM_GETCHECK, 0, 0));
    Settings->Set(SE_SHOWTIPSATSTARTUP, SendMessage(GetDlgItem(hWnd, IDCHECKBOX_SHOWTIPSATSTARTUP), BM_GETCHECK, 0, 0));
    Settings->Set(SE_WINDOW,            SendMessage(GetDlgItem(hWnd, IDCHECKBOX_WINDOW),            BM_GETCHECK, 0, 0));
}

static void __stdcall SetVideoTab(HWND hWnd)
{
    unsigned long i;
    HWND hChild;

    // feed the lists
    hChild = GetDlgItem(hWnd, IDCOMBOBOX_VIDEODEVICE);
    SendMessage(hChild, CB_RESETCONTENT, 0, 0);
    for(i = 0; i<l_DI.luItems; i++)
    {
        unsigned long* luGuid = (unsigned long*)&l_DI.Drivers[i].DriverGuid, k;
        bool bIsPrimary = !(luGuid[0] && luGuid[1] && luGuid[2] && luGuid[3]);

        for(k=0; k<l_DI.Drivers[i].luItems; k++)
        {
            SendMessage(hChild, CB_ADDSTRING, 0, (LPARAM)(bIsPrimary ? l_DI.Drivers[i].Devices[k].szName : l_DI.Drivers[i].szDescription));
        }
    }

    // refresh resolution
    OnChangeDevice(hWnd);

    // initialize track bars
    SendMessage(GetDlgItem(hWnd, IDTRACKBAR_SPRITEQ),  TBM_SETRANGE, (WPARAM)FALSE, (LPARAM)MAKELONG(0, 2));
    SendMessage(GetDlgItem(hWnd, IDTRACKBAR_TEXTUREQ), TBM_SETRANGE, (WPARAM)FALSE, (LPARAM)MAKELONG(0, 2));

    // reflect settings
    struct UIBatchList BatchList[] =
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
    struct UIBatchList BatchList[] =
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
    struct UIBatchList BatchList[] =
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
        { IDCHECKBOX_NOCTRL,            BM_SETCHECK, (WPARAM)Settings->Get(SE_NOCTRL),            0 },
        { IDCHECKBOX_NOSHIFT,           BM_SETCHECK, (WPARAM)Settings->Get(SE_NOSHIFT),           0 },
        { IDCHECKBOX_NOTRADE,           BM_SETCHECK, (WPARAM)Settings->Get(SE_NOTRADE),           0 },
        { IDCHECKBOX_SHOPPING,          BM_SETCHECK, (WPARAM)Settings->Get(SE_SHOPPING),          0 },
        { IDCHECKBOX_SNAP,              BM_SETCHECK, (WPARAM)Settings->Get(SE_SNAP),              0 },
        { IDCHECKBOX_SKILLFAIL,         BM_SETCHECK, (WPARAM)Settings->Get(SE_SKILLFAIL),         0 },
        { IDCHECKBOX_NOTALKMSG,         BM_SETCHECK, (WPARAM)Settings->Get(SE_NOTALKMSG),         0 },
        { IDCHECKBOX_NOTALKMSG2,        BM_SETCHECK, (WPARAM)Settings->Get(SE_NOTALKMSG2),        0 },
        { IDCHECKBOX_SHOWNAME,          BM_SETCHECK, (WPARAM)Settings->Get(SE_SHOWNAME),          0 },
        { IDCHECKBOX_STATEINFO,         BM_SETCHECK, (WPARAM)Settings->Get(SE_STATEINFO),         0 },
        { IDCHECKBOX_SHOWTIPSATSTARTUP, BM_SETCHECK, (WPARAM)Settings->Get(SE_SHOWTIPSATSTARTUP), 0 },
        { IDCHECKBOX_WINDOW,            BM_SETCHECK, (WPARAM)Settings->Get(SE_WINDOW),            0 },
    };
    UI::BatchMessage(hWnd, BatchList, __ARRAYSIZE(BatchList));

    // store availability
    DisableUnavailableSetting(SE_AURA, hWnd, IDCHECKBOX_AURA);
    DisableUnavailableSetting(SE_ISFIXEDCAMERA, hWnd, IDCHECKBOX_ISFIXEDCAMERA);
    DisableUnavailableSetting(SE_ISEFFECTON, hWnd, IDCHECKBOX_ISEFFECTON);
    DisableUnavailableSetting(SE_ONHOUSERAI, hWnd, IDCHECKBOX_ONHOUSERAI);
    DisableUnavailableSetting(SE_ISITEMSNAP, hWnd, IDCHECKBOX_ISITEMSNAP);
    DisableUnavailableSetting(SE_LOGINOUT, hWnd, IDCHECKBOX_LOGINOUT);
    DisableUnavailableSetting(SE_ONMERUSERAI, hWnd, IDCHECKBOX_ONMERUSERAI);
    DisableUnavailableSetting(SE_MAKEMISSEFFECT, hWnd, IDCHECKBOX_MAKEMISSEFFECT);
    DisableUnavailableSetting(SE_NOCTRL, hWnd, IDCHECKBOX_NOCTRL);
    DisableUnavailableSetting(SE_NOSHIFT, hWnd, IDCHECKBOX_NOSHIFT);
    DisableUnavailableSetting(SE_NOTRADE, hWnd, IDCHECKBOX_NOTRADE);
    DisableUnavailableSetting(SE_SHOPPING, hWnd, IDCHECKBOX_SHOPPING);
    DisableUnavailableSetting(SE_SNAP, hWnd, IDCHECKBOX_SNAP);
    DisableUnavailableSetting(SE_SKILLFAIL, hWnd, IDCHECKBOX_SKILLFAIL);
    DisableUnavailableSetting(SE_NOTALKMSG, hWnd, IDCHECKBOX_NOTALKMSG);
    DisableUnavailableSetting(SE_NOTALKMSG2, hWnd, IDCHECKBOX_NOTALKMSG2);
    DisableUnavailableSetting(SE_SHOWNAME, hWnd, IDCHECKBOX_SHOWNAME);
    DisableUnavailableSetting(SE_STATEINFO, hWnd, IDCHECKBOX_STATEINFO);
    DisableUnavailableSetting(SE_SHOWTIPSATSTARTUP, hWnd, IDCHECKBOX_SHOWTIPSATSTARTUP);
    DisableUnavailableSetting(SE_WINDOW, hWnd, IDCHECKBOX_WINDOW);
}

static BOOL CALLBACK VideoTabProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    unsigned long i;

    switch(uMsg)
    {
        case WM_COMMAND:
            switch(HIWORD(wParam))
            {
                case CBN_SELCHANGE:
                    if(LOWORD(wParam)==IDCOMBOBOX_VIDEODEVICE)
                    {
                        if((i = SendMessage((HWND)lParam, CB_GETCURSEL, 0, 0))!=CB_ERR)
                        {// required for update resolution
                            Settings->Set(SE_DEVICECNT, i);
                        }
                        OnChangeDevice(hWnd);
                    }
                    break;
                default:
                    return FALSE;
            }
            break;
        case WM_HELP:
            UI::HHLite(hWnd, (LPHELPINFO)lParam);
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
        case WM_HELP:
            UI::HHLite(hWnd, (LPHELPINFO)lParam);
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
        case WM_HELP:
            UI::HHLite(hWnd, (LPHELPINFO)lParam);
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
            break;
        case WM_CTLCOLORSTATIC:
            if(GetDlgItem(hWnd, IDC_ABOUT_CORPSE)!=(HWND)lParam)
            {// not the EDIT box
                return FALSE;
            }
            // make readonly/disabled edit box have white background
            return (BOOL)GetStockObject(WHITE_BRUSH);
        default:
            return FALSE;
    }
    return TRUE;
}

static void __stdcall OnChangeSettingsEngine(HWND hWnd)
{
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
}

static void __stdcall OnBackgroundSave(HWND hWnd, unsigned long luHash)
{
    char szSlave[MAX_PATH], szParam[128];
    SHELLEXECUTEINFO Sei = { sizeof(Sei), SEE_MASK_FLAG_NO_UI|SEE_MASK_NOCLOSEPROCESS, hWnd, "runas", szSlave, szParam, ".", SW_SHOWDEFAULT };

    GetModuleFileNameA(NULL, szSlave, __ARRAYSIZE(szSlave));
    wsprintfA(szParam, "/save:%lu,%lu", Settings->GetEngineID(), luHash);

    if(ShellExecuteEx(&Sei) && Sei.hProcess)
    {// FIXME: Handle the save without blocking the message queue
        WaitForSingleObject(Sei.hProcess, INFINITE);
        CloseHandle(Sei.hProcess);
    }
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
                (LPARAM)LoadImage(hInstance, MAKEINTRESOURCE(IDI_APPLICATION_LARGE), IMAGE_ICON, 32, 32, LR_SHARED));
            SendMessage(hWnd, WM_SETICON, ICON_SMALL,
                (LPARAM)LoadImage(hInstance, MAKEINTRESOURCE(IDI_APPLICATION_SMALL), IMAGE_ICON, 16, 16, LR_SHARED));

            // initialize tab manager
            if(TabMgr.Init(hInstance, hWnd, 16, MAKEINTRESOURCE(IDB_IMAGELIST32), MAKEINTRESOURCE(IDB_IMAGELIST), MAKEINTRESOURCE(IDB_IMAGELIST_MASK), IMI_MAX))
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
                // TODO: Add RO_MF support

                // about
                LoadStringA(hInstance, TEXT_TAB_ABOUT, szTabTitle, __ARRAYSIZE(szTabTitle));
                TabMgr.AddTab(szTabTitle, IDABOUT, IMI_ABOUT, false, MAKEINTRESOURCE(IDD_TAB_ABOUT), &AboutTabProc);
            }

            // load MUI
            UI::LoadMUI(hWnd);

            // load settings and refresh tabs
            OnChangeSettingsEngine(hWnd);

            // take over the tab font (to satisfy multibyte charsets)
            if(!l_DlgFont)
            {
                LOGFONT Lf;

                if(GetObject((HGDIOBJ)SendMessage(TabMgr.GetWindowHandle(), WM_GETFONT, 0, 0), sizeof(Lf), &Lf))
                {
                    HDC hDC;
                    UINT uFontSize;

                    switch(PRIMARYLANGID(LANGIDFROMLCID(GetThreadLocale())))
                    {
                        case LANG_CHINESE:
                        case LANG_JAPANESE:
                        case LANG_KOREAN:
                            // account for kanji/hangul readability
                            uFontSize = DLG_FONT_SIZE+1;
                            break;
                        default:
                            uFontSize = DLG_FONT_SIZE;
                            break;
                    }

                    hDC = GetDC(hWnd);
                    Lf.lfHeight = -MulDiv(uFontSize, GetDeviceCaps(hDC, LOGPIXELSY), 72);
                    ReleaseDC(hWnd, hDC);

                    l_DlgFont = CreateFontIndirect(&Lf);
                }
            }
            if(l_DlgFont)
            {
                UI::SetFont(hWnd, l_DlgFont);
            }

            return TRUE;
        }
        case WM_COMMAND:
            switch(HIWORD(wParam))
            {
                case 0:
                    switch(LOWORD(wParam))
                    {
                        case IDOK:
                        case IDAPPLY:
                            ROExt.Save();

                            // collect data from tabs
                            GetVideoTab(TabMgr.GetTab(IDVIDEO));
                            GetSoundTab(TabMgr.GetTab(IDSOUND));
                            GetSetupTab(TabMgr.GetTab(IDSETUP));

                            // persist storage
                            if(Settings->IsAdminRequired())
                            {
                                OnBackgroundSave(hWnd, Settings->SaveToIPC());
                            }
                            else
                            {
                                Settings->Save();
                            }
                            // FIXME: Do not OK-quit on save/ipc failure

                            if(LOWORD(wParam)==IDAPPLY)
                            {// do not quit when 'Apply'
                                break;
                            }
                            // but fall through for 'OK'
                        case IDCANCEL:
                            EndDialog(hWnd, LOWORD(wParam));
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
                                    SendMessage(GetDlgItem(hWnd, IDELOGO), STM_SETIMAGE, IMAGE_ICON, (LPARAM)LoadImage(hInstance, MAKEINTRESOURCE(IDI_ENGINE_REG), IMAGE_ICON, 0, 0, LR_SHARED));
                                    break;
                                case SENGINE_REG:
                                    Settings = &SettingsLua;
                                    SendMessage(GetDlgItem(hWnd, IDELOGO), STM_SETIMAGE, IMAGE_ICON, (LPARAM)LoadImage(hInstance, MAKEINTRESOURCE(IDI_ENGINE_LUA), IMAGE_ICON, 0, 0, LR_SHARED));
                                    break;
                            }

                            // load settings and refresh tabs
                            OnChangeSettingsEngine(hWnd);
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

static int __cdecl DX7E_P_GetInfoSort(const void* lpA, const void* lpB)
{
    struct DDrawModeEntry* lpModeA = (struct DDrawModeEntry*)lpA;
    struct DDrawModeEntry* lpModeB = (struct DDrawModeEntry*)lpB;

    if(lpModeA->luWidth==lpModeB->luWidth)
    {
        return lpModeA->luHeight-lpModeB->luHeight;
    }
    return lpModeA->luWidth-lpModeB->luWidth;
}

static bool __stdcall DX7E_P_GetInfo(void)
{
    unsigned long luDrivers, luModes;

    // retrieve direct x information
    if(!DX7E_EnumDriverDevices(&l_DI))
    {
        return false;
    }

    // sort out irrelevant stuff
    for(luDrivers=0; luDrivers<l_DI.luItems; luDrivers++)
    {
        for(luModes = l_DI.Drivers[luDrivers].luModes; luModes>0; luModes--)
        {
            if(l_DI.Drivers[luDrivers].Modes[luModes-1].luWidth<640 || l_DI.Drivers[luDrivers].Modes[luModes-1].luHeight<480 || l_DI.Drivers[luDrivers].Modes[luModes-1].luBitDepth!=16)
            {// setup imposed limitations
                if(luModes<l_DI.Drivers[luDrivers].luModes)
                {
                    MoveMemory(&l_DI.Drivers[luDrivers].Modes[luModes-1], &l_DI.Drivers[luDrivers].Modes[luModes], (l_DI.Drivers[luDrivers].luModes-luModes)*sizeof(struct DDrawModeEntry));
                }
                l_DI.Drivers[luDrivers].luModes--;
            }
        }
        if(l_DI.Drivers[luDrivers].luModes)
        {// setup sorts modes
            qsort(l_DI.Drivers[luDrivers].Modes, l_DI.Drivers[luDrivers].luModes, sizeof(struct DDrawModeEntry), &DX7E_P_GetInfoSort);
        }
    }

    return true;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, char* lpCmdLine, int nShowCmd)
{
    // handle ipc save request
    if(lpCmdLine[0]=='/')
    {
        unsigned long luEngine = 0, luHash = 0;

        if(sscanf(lpCmdLine, "/save:%lu,%lu", &luEngine, &luHash)==2)
        {
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
            Settings->Save();
        }

        return EXIT_SUCCESS;
    }

    // Handle Windows8's sick PCA, which creates compatibility
    // entries for pretty much every app, thus breaks, rather
    // than fixing them.
    // This can be removed when Gravity decides to run RO in 32-bit
    // color modes.
    for(;;)
    {
        CHAR szFileName[MAX_PATH], szCompatFlags[1024];
        DWORD dwDisposition, dwLength = 0;
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

        lResult = RegCreateKeyExA(
            HKEY_CURRENT_USER,                                                          // handle this per user
            "Software\\Microsoft\\Windows NT\\CurrentVersion\\AppCompatFlags\\Layers",  // PCA compatibility flags
            0,                                                                          // reserved
            NULL,                                                                       // no key class
            REG_OPTION_NON_VOLATILE,                                                    // persist to disk
            KEY_READ|KEY_WRITE,                                                         // read & write
            NULL,                                                                       // default security (inherit)
            &hKey,                                                                      // destination key handle
            &dwDisposition                                                              // 'created' or 'opened' disposition
        );

        if(lResult!=ERROR_SUCCESS)
        {// restricted access?
            break;
        }

        if(dwDisposition==REG_OPENED_EXISTING_KEY)
        {// key already existed
            REGUTILLOADINFO Li[] =
            {
                { szFileName, szCompatFlags, __ARRAYSIZE(szCompatFlags), &dwLength, REG_SZ },
            };

            if(RegUtilLoad(hKey, Li, __ARRAYSIZE(Li)))
            {
                if(szCompatFlags[0])
                {// there are flags
                    if(!lstrcmpiA(szCompatFlags, "~ 16BITCOLOR"))
                    {// there are the flags we intend to set
                        RegCloseKey(hKey);
                        break;
                    }
                }
            }
        }

        REGUTILSAVEINFO Si[] =
        {
            { szFileName, "~ 16BITCOLOR", 12+1, REG_SZ },
        };
        RegUtilSave(hKey, Si, __ARRAYSIZE(Si));
        RegCloseKey(hKey);

        // restart
        SHELLEXECUTEINFO Sei = { sizeof(Sei), 0, NULL, "open", szFileName, "", ".", SW_SHOWDEFAULT };

        if(ShellExecuteExA(&Sei) && Sei.hProcess)
        {
            CloseHandle(Sei.hProcess);
        }

        return EXIT_SUCCESS;
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

    // show 'em the dialog
    DialogBox(hInstance, MAKEINTRESOURCE(IDD_MAIN_DIALOG), NULL, &MainDialogProc);

    // free font, if any
    if(l_DlgFont)
    {
        DeleteObject(l_DlgFont);
        l_DlgFont = NULL;
    }

    // release mutex
    CloseHandle(hMutex);

    return EXIT_SUCCESS;
}
