// -----------------------------------------------------------------
// RagnarokOnline OpenSetup
// (c) 2010 Ai4rei/AN
// See doc/license.txt for details.
// -----------------------------------------------------------------

#include "snippets/btypes.h"
#include "snippets/cpenum.h"
#include "snippets/cstr.h"


#define DIRECT3D_VERSION 0x0700
#define DIRECTDRAW_VERSION 0x0700
#include <windows.h>
#include <commctrl.h>
#include <d3d.h>
#include <ddraw.h>

#include "dx7enum.h"
#include "resource.h"
#include "roext.h"
#include "settings.h"
#include "tab.h"
#include "ui.h"

#include "opensetup.h"

static const char* lpszSignature = "$ RagnarokOnline OpenSetup, "APP_VERSION", build on "__DATE__" @ "__TIME__", (c) 2010 Ai4rei/AN $";

static const char* lpszNumSampleTypes[] =
{
    "48 Channel",
    "32 Channel",
    "16 Channel",
};
static const char* lpszSoundModes[] =
{
    "No Sound",
    "Use 2D Sound",
    "Use 3D Sound",
};
static const char* lpszSpeakerTypes[] =
{
    "2 SPEAKER",
    "HEADPHONE",
    "SURROUND",
    "4 SPEAKER",
};
static const char* lpszDigitalRateTypes[] =
{
    "22k",
    "11k",
    "8k",
};
static const char* lpszDigitalBitsTypes[] =
{
    "16 Bit",
    "8 Bit",
};

static struct DDrawDriverDeviceInfo g_DI;
static unsigned long luPrimaryDevices = 0;
static class CROExt ROExt;
static class CSettings Settings;
static class CTabMgr TabMgr;

static HWND hTabVideo = NULL, hTabSound = NULL;

inline unsigned long GetDriverIndex(unsigned long luComboIndex)
{
    return (luComboIndex<luPrimaryDevices) ? 0 : luComboIndex-luPrimaryDevices+1;
}

inline unsigned long GetDeviceIndex(unsigned long luComboIndex)
{
    return (luComboIndex<luPrimaryDevices) ? luComboIndex : 0 /* listed drivers default to Device 0 */;
}

static void __stdcall OnChangeDevice(HWND hWnd)
{
    char szLabel[128];
    unsigned long i, lResult, luIndex;
    struct DDrawDriverEntry* lpEntry;
    HWND hResWnd;

    hResWnd = GetDlgItem(hWnd, IDCOMBOBOX_RESOLUTION);
    lResult = Settings.Get(SE_DEVICECNT);
    luIndex = GetDriverIndex(lResult);

    if(luIndex>=g_DI.luItems)
    {// ignore
        return;
    }
    lpEntry = &g_DI.Drivers[luIndex];

    // save selection
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

        Settings.Set(SE_DEVICECNT, i);
        Settings.Set(SE_GUIDDRIVER, &g_DI.Drivers[luIndex].DriverGuid);
        Settings.Set(SE_GUIDDEVICE, &g_DI.Drivers[luIndex].Devices[GetDeviceIndex(i)].DeviceGuid);
        Settings.Set(SE_DEVICENAME,  g_DI.Drivers[luIndex].Devices[GetDeviceIndex(i)].szName);
    }
    if((i = SendMessage(GetDlgItem(hWnd, IDCOMBOBOX_RESOLUTION), CB_GETCURSEL, 0, 0))!=CB_ERR)
    {
        unsigned long luIndex = GetDriverIndex(Settings.Get(SE_DEVICECNT));

        Settings.Set(SE_MODECNT,     i);
        Settings.Set(SE_WIDTH,       g_DI.Drivers[luIndex].Modes[i].luWidth   );
        Settings.Set(SE_HEIGHT,      g_DI.Drivers[luIndex].Modes[i].luHeight  );
        Settings.Set(SE_BITPERPIXEL, g_DI.Drivers[luIndex].Modes[i].luBitDepth);
    }

    // check boxes
    Settings.Set(SE_ISFULLSCREENMODE, SendMessage(GetDlgItem(hWnd, IDCHECKBOX_FULLSCREEN), BM_GETCHECK, 0, 0));
    Settings.Set(SE_FOG,              SendMessage(GetDlgItem(hWnd, IDCHECKBOX_FOG),        BM_GETCHECK, 0, 0));
    Settings.Set(SE_ISLIGHTMAP,       SendMessage(GetDlgItem(hWnd, IDCHECKBOX_LIGHTMAP),   BM_GETCHECK, 0, 0));
    Settings.Set(SE_ISVOODOO,         SendMessage(GetDlgItem(hWnd, IDCHECKBOX_VOODOO),     BM_GETCHECK, 0, 0));

    // track bars
    Settings.Set(SE_SPRITEMODE,       SendMessage(GetDlgItem(hWnd, IDTRACKBAR_SPRITEQ),    TBM_GETPOS,  0, 0));
    Settings.Set(SE_TEXTUREMODE,      SendMessage(GetDlgItem(hWnd, IDTRACKBAR_TEXTUREQ),   TBM_GETPOS,  0, 0));
}

static void __stdcall GetSoundTab(HWND hWnd)
{
    unsigned long i;

    // simple combo boxes
    if((i = SendMessage(GetDlgItem(hWnd, IDCOMBOBOX_SOUND_MODE), CB_GETCURSEL, 0, 0))!=CB_ERR)
    {
        Settings.Set(SE_SOUNDMODE, i);
    }
    if((i = SendMessage(GetDlgItem(hWnd, IDCOMBOBOX_SOUND_SPEAKERTYPE), CB_GETCURSEL, 0, 0))!=CB_ERR)
    {
        Settings.Set(SE_SPEAKERTYPE, i);
    }
    if((i = SendMessage(GetDlgItem(hWnd, IDCOMBOBOX_SOUND_BITRATE), CB_GETCURSEL, 0, 0))!=CB_ERR)
    {
        Settings.Set(SE_DIGITALRATETYPE, i);
    }
    if((i = SendMessage(GetDlgItem(hWnd, IDCOMBOBOX_SOUND_BITDEPTH), CB_GETCURSEL, 0, 0))!=CB_ERR)
    {
        Settings.Set(SE_DIGITALBITSTYPE, i);
    }
    if((i = SendMessage(GetDlgItem(hWnd, IDCOMBOBOX_SOUND_CHANNELS), CB_GETCURSEL, 0, 0))!=CB_ERR)
    {
        Settings.Set(SE_NUMSAMPLETYPE, i);
    }
}

static void __stdcall SetVideoTab(HWND hWnd)
{
    unsigned long i;
    HWND hChild;

    // feed the lists
    hChild = GetDlgItem(hWnd, IDCOMBOBOX_VIDEODEVICE);
    for(i = 0; i<g_DI.luItems; i++)
    {
        unsigned long* luGuid = (unsigned long*)&g_DI.Drivers[i].DriverGuid, k;

        if(luGuid[0]+luGuid[1]+luGuid[2]+luGuid[3])
        {
            SendMessage(hChild, CB_ADDSTRING, 0, (LPARAM)g_DI.Drivers[i].szDescription);
        }
        else
        {
            for(k=0; k<g_DI.Drivers[i].luItems; k++)
            {
                SendMessage(hChild, CB_ADDSTRING, 0, (LPARAM)g_DI.Drivers[i].Devices[k].szName);
            }
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
        { IDCHECKBOX_FULLSCREEN,  BM_SETCHECK,  (WPARAM)Settings.Get(SE_ISFULLSCREENMODE), 0                                    },
        { IDCHECKBOX_FOG,         BM_SETCHECK,  (WPARAM)Settings.Get(SE_FOG),              0                                    },
        { IDCHECKBOX_LIGHTMAP,    BM_SETCHECK,  (WPARAM)Settings.Get(SE_ISLIGHTMAP),       0                                    },
        { IDCHECKBOX_VOODOO,      BM_SETCHECK,  (WPARAM)Settings.Get(SE_ISVOODOO),         0                                    },
        // initialize track bars
        { IDTRACKBAR_SPRITEQ,     TBM_SETPOS,   (WPARAM)TRUE,                              (LPARAM)Settings.Get(SE_SPRITEMODE)  },
        { IDTRACKBAR_TEXTUREQ,    TBM_SETPOS,   (WPARAM)TRUE,                              (LPARAM)Settings.Get(SE_TEXTUREMODE) },
        // initialize combo boxes
        { IDCOMBOBOX_VIDEODEVICE, CB_SETCURSEL, (WPARAM)Settings.Get(SE_DEVICECNT),        0                                    },
        { IDCOMBOBOX_RESOLUTION,  CB_SETCURSEL, (WPARAM)Settings.Get(SE_MODECNT),          0                                    },
    };
    UI::BatchMessage(hWnd, BatchList, ARRAYLEN(BatchList));
}

static void __stdcall SetSoundTab(HWND hWnd)
{
    // initialize static combo boxes
    UI::FillComboBox(hWnd, IDCOMBOBOX_SOUND_MODE,        lpszSoundModes,       ARRAYLEN(lpszSoundModes)      );
    UI::FillComboBox(hWnd, IDCOMBOBOX_SOUND_SPEAKERTYPE, lpszSpeakerTypes,     ARRAYLEN(lpszSpeakerTypes)    );
    UI::FillComboBox(hWnd, IDCOMBOBOX_SOUND_BITRATE,     lpszDigitalRateTypes, ARRAYLEN(lpszDigitalRateTypes));
    UI::FillComboBox(hWnd, IDCOMBOBOX_SOUND_BITDEPTH,    lpszDigitalBitsTypes, ARRAYLEN(lpszDigitalBitsTypes));
    UI::FillComboBox(hWnd, IDCOMBOBOX_SOUND_CHANNELS,    lpszNumSampleTypes,   ARRAYLEN(lpszNumSampleTypes)  );

    // reflect settings
    struct UIBatchList BatchList[] =
    {
        // initialize combo boxes
        { IDCOMBOBOX_SOUND_MODE,        CB_SETCURSEL, (WPARAM)Settings.Get(SE_SOUNDMODE),       0 },
        { IDCOMBOBOX_SOUND_SPEAKERTYPE, CB_SETCURSEL, (WPARAM)Settings.Get(SE_SPEAKERTYPE),     0 },
        { IDCOMBOBOX_SOUND_BITRATE,     CB_SETCURSEL, (WPARAM)Settings.Get(SE_DIGITALRATETYPE), 0 },
        { IDCOMBOBOX_SOUND_BITDEPTH,    CB_SETCURSEL, (WPARAM)Settings.Get(SE_DIGITALBITSTYPE), 0 },
        { IDCOMBOBOX_SOUND_CHANNELS,    CB_SETCURSEL, (WPARAM)Settings.Get(SE_NUMSAMPLETYPE),   0 },
    };
    UI::BatchMessage(hWnd, BatchList, ARRAYLEN(BatchList));
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
                            Settings.Set(SE_DEVICECNT, i);
                        }
                        OnChangeDevice(hWnd);
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

static BOOL CALLBACK SoundTabProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    return FALSE;
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

            // recall storage
            Settings.Load();

            // initialize tab manager
            if(TabMgr.Init(hInstance, hWnd))
            {
                hTabVideo = TabMgr.AddTab("Graphics", IDVIDEO, IMI_VIDEO, true, MAKEINTRESOURCE(IDD_TAB_VIDEO), &VideoTabProc);
                hTabSound = TabMgr.AddTab("Sounds", IDSOUND, IMI_SOUND, false, MAKEINTRESOURCE(IDD_TAB_SOUND), &SoundTabProc);

                // initialize tabs
                SetVideoTab(hTabVideo);
                SetSoundTab(hTabSound);

                // add-ons
                ROExt.Load(&TabMgr);

                // about
                TabMgr.AddButton("About", IDABOUT, IMI_ABOUT);
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
                            GetVideoTab(hTabVideo);
                            GetSoundTab(hTabSound);

                            // persist storage
                            Settings.Save();

                            if(LOWORD(wParam)==IDAPPLY)
                            {// do not quit when 'Apply'
                                break;
                            }
                            // but fall through for 'OK'
                        case IDCANCEL:
                            EndDialog(hWnd, LOWORD(wParam));
                            break;
                        case IDVIDEO:
                            ROExt.Show(SW_HIDE);
                            ShowWindow(hTabSound, SW_HIDE);
                            ShowWindow(hTabVideo, SW_SHOW);
                            break;
                        case IDSOUND:
                            ROExt.Show(SW_HIDE);
                            ShowWindow(hTabVideo, SW_HIDE);
                            ShowWindow(hTabSound, SW_SHOW);
                            break;
                        case IDROEXT:
                            ShowWindow(hTabVideo, SW_HIDE);
                            ShowWindow(hTabSound, SW_HIDE);
                            ROExt.Show(SW_SHOW);
                            break;
                        case IDABOUT:
                            UI::MessageBoxEx(hWnd, APP_DLGABOUT, "About RagnarokOnline OpenSetup...", MB_OK);
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
    ZeroMemory(&g_DI, sizeof(g_DI));
    if(!DX7E_EnumDriverDevices(&g_DI))
    {
        return false;
    }

    // sort out irrelevant stuff
    for(luDrivers=0; luDrivers<g_DI.luItems; luDrivers++)
    {
        unsigned long* lpGuid = (unsigned long*)&g_DI.Drivers[luDrivers].DriverGuid;

        if(lpGuid[0]+lpGuid[1]+lpGuid[2]+lpGuid[3])
        {
            ;
        }
        else
        {
            // instead of primary display driver, all available
            // primary display driver devices are shown.
            luPrimaryDevices = g_DI.Drivers[luDrivers].luItems;
        }
        for(luModes = g_DI.Drivers[luDrivers].luModes; luModes>0; luModes--)
        {
            if(g_DI.Drivers[luDrivers].Modes[luModes-1].luHeight<480 || g_DI.Drivers[luDrivers].Modes[luModes-1].luBitDepth!=16)
            {// setup imposed limitations
                if(luModes<g_DI.Drivers[luDrivers].luModes)
                {
                    MoveMemory(&g_DI.Drivers[luDrivers].Modes[luModes-1], &g_DI.Drivers[luDrivers].Modes[luModes], (g_DI.Drivers[luDrivers].luModes-luModes)*sizeof(struct DDrawModeEntry));
                }
                g_DI.Drivers[luDrivers].luModes--;
            }
        }
        if(g_DI.Drivers[luDrivers].luModes)
        {// setup sorts modes
            qsort(g_DI.Drivers[luDrivers].Modes, g_DI.Drivers[luDrivers].luModes, sizeof(struct DDrawModeEntry), &DX7E_P_GetInfoSort);
        }
    }

    return true;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, char* lpCmdLine, int nShowCmd)
{
    INITCOMMONCONTROLSEX icce = { sizeof(icce), ICC_WIN95_CLASSES };

    // init common controls
    if(!InitCommonControlsEx(&icce))
    {
        MessageBox(NULL, "Failed to InitCommonControlsEx.", "Error", MB_OK|MB_ICONSTOP);
        return 1;
    }

    // init direct x information
    if(!DX7E_P_GetInfo())
    {
        MessageBox(NULL, "Failed to DX7E_P_GetInfo.", "Error", MB_OK|MB_ICONSTOP);
        return 1;
    }

    // show 'em the dialog
    DialogBox(hInstance, MAKEINTRESOURCE(IDD_MAIN_DIALOG), NULL, &MainDialogProc);
    return 0;
}
