// -----------------------------------------------------------------
// RagnarokOnline OpenSetup
// (c) 2010 Ai4rei/AN
// See doc/license.txt for details.
// -----------------------------------------------------------------

#ifndef _RESOURCE_H_
#define _RESOURCE_H_

// Icons
#define IDI_APPLICATION_LARGE           1
#define IDI_APPLICATION_SMALL           2

// Bitmaps
#define IDB_LOGO                        2001
#define IDB_IMAGELIST                   2002
#define IDB_IMAGELIST_MASK              2003

// Imagelist Indexes
#define IMI_VIDEO                       0
#define IMI_SOUND                       1
#define IMI_PLUGS                       2
#define IMI_ABOUT                       3
#define IMI_MAX                         4

// Dialogs
#define IDD_MAIN_DIALOG                 100
#define IDD_TAB_MANAGER                 101  // Not really a dialog.
#define IDD_TAB_VIDEO                   102
#define IDD_TAB_SOUND                   103
#define IDD_TAB_ROEXT                   104

// Controls (Video & Sound)
#define IDCOMBOBOX_VIDEODEVICE          1000
#define IDCOMBOBOX_RESOLUTION           1001
#define IDCOMBOBOX_SOUND_MODE           1007
#define IDCOMBOBOX_SOUND_SPEAKERTYPE    1008
#define IDCOMBOBOX_SOUND_BITRATE        1010
#define IDCOMBOBOX_SOUND_BITDEPTH       1011
#define IDCOMBOBOX_SOUND_CHANNELS       1017
#define IDCHECKBOX_FULLSCREEN           1014
#define IDCHECKBOX_FOG                  1018
#define IDCHECKBOX_LIGHTMAP             1013
#define IDCHECKBOX_VOODOO               1012
#define IDTRACKBAR_SPRITEQ              1016
#define IDTRACKBAR_TEXTUREQ             1015

// Controls (ROExt)
#define IDCHECKBOX_MOUSEFREEDOM         1000
#define IDCHECKBOX_REMAPMOUSEBUTTONS    1001
#define IDCHECKBOX_REMAPALTF4           1002
#define IDCHECKBOX_AUTOFREECPU          1003
#define IDCHECKBOX_WINDOWONTOP          1004
#define IDCHECKBOX_WINDOWLOCK           1005
#define IDCOMBOBOX_CODEPAGE             1006

// Buttons
#define IDAPPLY                         1201
#define IDVIDEO                         1202
#define IDSOUND                         1203
#define IDROEXT                         1204
#define IDABOUT                         1205
#define IDLLOGO                         1206

#define APP_VERSION "1.1.2.50"
#define APP_VERSIONINFO_VERSION 1,1,2,50

// About
#define APP_DLGABOUT "RagnarokOnline OpenSetup, Version "APP_VERSION"-open\r\n(C) 2010 Ai4rei/AN\r\nhttp:/\x2fnn.nachtwolke.com/dev/opensetup/\r\n\r\n"\
    "RagnarokOnline, Ragnarok-related graphics and materials are copyright (C) 2002-2010 Gravity Co., Ltd. & Lee Myoungjin.\r\n\r\n"\
    "Tab icons are taken from the \"LED Icon Set\" v1.0 - http:/\x2fled24.de/iconset/\r\n\r\n"\
    "This software is FREEWARE and is provided AS IS, without warranty of ANY KIND, either expressed or implied, including but not limited to the implied warranties of merchantability and/or fitness for a particular purpose. "\
    "The author SHALL NOT be held liable for ANY damage to you, your hardware, your software, or to anyone or anything else, that may result from its use, or misuse. Basically, you use it at YOUR OWN RISK."

#endif  /* _RESOURCE_H_ */
