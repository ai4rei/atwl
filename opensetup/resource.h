// -----------------------------------------------------------------
// RagnarokOnline OpenSetup
// (c) 2010-2013 Ai4rei/AN
// See doc/license.txt for details.
//
// -----------------------------------------------------------------

#ifndef _RESOURCE_H_
#define _RESOURCE_H_

// Icons
#define IDI_APPLICATION_LARGE           1
#define IDI_APPLICATION_SMALL           2
#define IDI_ENGINE_REG                  3
#define IDI_ENGINE_LUA                  4

// Bitmaps
#define IDB_LOGO                        2001
#define IDB_IMAGELIST32                 2002
#define IDB_IMAGELIST                   2003
#define IDB_IMAGELIST_MASK              2004

// Imagelist Indexes
#define IMI_VIDEO                       0
#define IMI_SOUND                       1
#define IMI_SETUP                       2
#define IMI_PLUGS                       3
#define IMI_ABOUT                       4
#define IMI_MAX                         5

// Dialogs
#define IDD_MAIN_DIALOG                 100
#define IDD_TAB_MANAGER                 101  // Not really a dialog.
#define IDD_TAB_VIDEO                   102
#define IDD_TAB_SOUND                   103
#define IDD_TAB_SETUP                   104
#define IDD_TAB_ROEXT                   105
#define IDD_TAB_ABOUT                   106

// Controls (Video, Sound, Setup)
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
#define IDCHECKBOX_AURA                 1019
#define IDCHECKBOX_ISFIXEDCAMERA        1020
#define IDCHECKBOX_ISEFFECTON           1021
#define IDCHECKBOX_ONHOUSERAI           1022
#define IDCHECKBOX_ISITEMSNAP           1023
#define IDCHECKBOX_LOGINOUT             1024
#define IDCHECKBOX_ONMERUSERAI          1025
#define IDCHECKBOX_MAKEMISSEFFECT       1026
#define IDCHECKBOX_NOCTRL               1027
#define IDCHECKBOX_NOSHIFT              1028
#define IDCHECKBOX_NOTRADE              1029
#define IDCHECKBOX_SHOPPING             1030
#define IDCHECKBOX_SNAP                 1031
#define IDCHECKBOX_SKILLFAIL            1032
#define IDCHECKBOX_NOTALKMSG            1033
#define IDCHECKBOX_NOTALKMSG2           1034
#define IDCHECKBOX_SHOWNAME             1035
#define IDCHECKBOX_STATEINFO            1036
#define IDCHECKBOX_SHOWTIPSATSTARTUP    1037
#define IDCHECKBOX_WINDOW               1038
#define IDTRACKBAR_STREAMVOLUME         1039
#define IDTRACKBAR_SOUNDVOLUME          1040
#define IDCHECKBOX_BGMISPAUSED          1041
#define IDCHECKBOX_ISSOUNDON            1042
#define IDCHECKBOX_MOUSEEXCLUSIVE       1043
#define IDCHECKBOX_TRILINEARFILTER      1044

// Controls (ROExt)
#define IDCHECKBOX_MOUSEFREEDOM         1000
#define IDCHECKBOX_REMAPMOUSEBUTTONS    1001
#define IDCHECKBOX_REMAPALTF4           1002
#define IDCHECKBOX_AUTOFREECPU          1003
#define IDCHECKBOX_WINDOWONTOP          1004
#define IDCHECKBOX_WINDOWLOCK           1005
#define IDCOMBOBOX_CODEPAGE             1006

// Controls (About)
#define IDC_ABOUT_TITLE                 1000
#define IDC_ABOUT_CORPSE                1001

// Buttons
#define IDAPPLY                         1201
#define IDVIDEO                         1202
#define IDSOUND                         1203
#define IDSETUP                         1204
#define IDROEXT                         1205
#define IDABOUT                         1206
#define IDLLOGO                         1207
#define IDELOGO                         1208

// Combobox Items
#define TEXT_NUMSAMPLETYPE_48CHANNEL    8000
#define TEXT_NUMSAMPLETYPE_32CHANNEL    8001
#define TEXT_NUMSAMPLETYPE_16CHANNEL    8002
#define TEXT_SOUNDMODE_NOSOUND          8003
#define TEXT_SOUNDMODE_2DSOUND          8004
#define TEXT_SOUNDMODE_3DSOUND          8005
#define TEXT_SPEAKERTYPE_2SPEAKER       8006
#define TEXT_SPEAKERTYPE_HEADPHONE      8007
#define TEXT_SPEAKERTYPE_SURROUND       8008
#define TEXT_SPEAKERTYPE_4SPEAKER       8009
#define TEXT_DIGITALRATETYPE_22K        8010
#define TEXT_DIGITALRATETYPE_11K        8011
#define TEXT_DIGITALRATETYPE_8K         8012
#define TEXT_DIGITALBITSTYPE_16BIT      8013
#define TEXT_DIGITALBITSTYPE_8BIT       8014

// Tabs
#define TEXT_TAB_GRAPHICS               8100
#define TEXT_TAB_SOUNDS                 8101
#define TEXT_TAB_SETTINGS               8102
#define TEXT_TAB_ABOUT                  8103

// Dialogs
#define TEXT_DLG_OK                     8200
#define TEXT_DLG_CANCEL                 8201
#define TEXT_DLG_APPLY                  8202
#define TEXT_DLG_GRAPHIC_SETTINGS       8203
#define TEXT_DLG_GRAPHIC_DEVICE         8204
#define TEXT_DLG_RESOLUTION             8205
#define TEXT_DLG_OPTIONS                8206
#define TEXT_DLG_FULLSCREEN             8207
#define TEXT_DLG_FOG                    8208
#define TEXT_DLG_LIGHTMAP               8209
#define TEXT_DLG_TRILINEAR_FILTER       8210
#define TEXT_DLG_SPRITE_QUALITY         8211
#define TEXT_DLG_TEXTURE_QUALITY        8212
#define TEXT_DLG_TROUBLESHOOTING        8213
#define TEXT_DLG_VOODOO3_CARD           8214
#define TEXT_DLG_RESTRICT_MOUSE         8215
#define TEXT_DLG_SOUND_SETTINGS         8216
#define TEXT_DLG_SOUND_MODE             8217
#define TEXT_DLG_SPEAKER_TYPE           8218
#define TEXT_DLG_CHANNELS               8219
#define TEXT_DLG_BITDEPTH               8220
#define TEXT_DLG_BITRATE                8221
#define TEXT_DLG_BGM_VOLUME             8222
#define TEXT_DLG_SOUND_VOLUME           8223
#define TEXT_DLG_MUTE                   8224
#define TEXT_DLG_GENERAL_SETTINGS       8225
#define TEXT_DLG_CMD_AURA               8226
#define TEXT_DLG_CMD_CAMERA             8227
#define TEXT_DLG_CMD_EFFECT             8228
#define TEXT_DLG_CMD_HOAI               8229
#define TEXT_DLG_CMD_ITEMSNAP           8230
#define TEXT_DLG_CMD_LOGINOUT           8231
#define TEXT_DLG_CMD_MERAI              8232
#define TEXT_DLG_CMD_MISS               8233
#define TEXT_DLG_CMD_NOCTRL             8234
#define TEXT_DLG_CMD_NOSHIFT            8235
#define TEXT_DLG_CMD_NOTRADE            8236
#define TEXT_DLG_CMD_SHOPPING           8237
#define TEXT_DLG_CMD_SNAP               8238
#define TEXT_DLG_CMD_SKILLFAIL          8239
#define TEXT_DLG_CMD_NOTALKMSG          8240
#define TEXT_DLG_CMD_NOTALKMSG2         8241
#define TEXT_DLG_CMD_SHOWNAME           8242
#define TEXT_DLG_CMD_STATEINFO          8243
#define TEXT_DLG_CMD_TOP                8244
#define TEXT_DLG_CMD_WINDOW             8245
#define TEXT_DLG_ROEXT_SETTINGS         8246
#define TEXT_DLG_MOUSE_FREEDOM          8247
#define TEXT_DLG_REMAP_MOUSE            8248
#define TEXT_DLG_REMAP_ALTF4            8249
#define TEXT_DLG_FREE_CPU               8250
#define TEXT_DLG_ALWAYS_ON_TOP          8251
#define TEXT_DLG_LOCK_WINDOW            8252
#define TEXT_DLG_CODEPAGE               8253
#define TEXT_DLG_ROEXT_DETECT_NOTE      8254
#define TEXT_DLG_ABOUT_OPENSETUP        8255
#define TEXT_DLG_LIST_CP_DEFAULT        8256
#define TEXT_DLG_LIST_CP_UNKNOWN        8257

// Error Messages
#define TEXT_ERROR__TITLE               9000
#define TEXT_ERROR__TITLE_LUA           9001
#define TEXT_ERROR__SYSTEM_ERROR        9002
#define TEXT_ERROR_INIT_MUTEX           9003
#define TEXT_ERROR_INIT_COMCTL32        9004
#define TEXT_ERROR_INIT_DIRECTX7        9005
#define TEXT_ERROR_HKEY_CREATE          9006
#define TEXT_ERROR_HKEY_WRITE           9007
#define TEXT_ERROR_HKEY_OPT_CREATE      9008
#define TEXT_ERROR_HKEY_OPT_WRITE       9009
#define TEXT_ERROR_FILE_WRITE           9010
#define TEXT_ERROR_FILE_OPEN_READONLY   9011
#define TEXT_ERROR_FILE_OPEN            9012
#define TEXT_ERROR_DIRECTORY_CREATE     9013
#define TEXT_ERROR_ROEXT_WRITE          9014

// Font
#define DLG_FONT_NAME "Tahoma"
#define DLG_FONT_SIZE 8

#define APP_VERSION "2.6.4.204"
#define APP_VERSIONINFO_VERSION 2,6,4,204
#define WITHOUT_BANNER

// About
#define APP_TITLE "RagnarokOnline OpenSetup, Version "APP_VERSION"\r\n(C) 2010-2013 Ai4rei/AN"
#define APP_CORPSE \
    "-=- Disclaimer -=-\r\n\r\n"\
    "This software is FREEWARE and is provided AS IS, without warranty of ANY KIND, either expressed or implied, including but not limited to the implied warranties of merchantability and/or fitness for a particular purpose. If your country's law does not allow complete exclusion of liability, you may not use this software. The author SHALL NOT be held liable for ANY damage to you, your hardware, your software, your pets, your dear other, or to anyone or anything else, that may or may not result from the use or misuse of this software. Basically, you use it at YOUR OWN RISK.\r\n\r\n"\
    "-=- Credits -=-\r\n\r\n"\
    "RagnarokOnline, Ragnarok-related graphics and materials are Copyright (C) 2002-2013 Gravity Co., Ltd. & Lee Myoungjin.\r\n\r\n"\
    "Tab icons are taken from the \"LED Icon Set\" v1.0\r\nhttp:/\x2fled24.de/iconset/\r\n\r\n"\
    "Uses scripting language Lua 5.1.4\r\nCopyright (C) 1994-2008 Lua.org, PUC-Rio.\r\n\r\n"\
    "Some texts are based on the msgstringtable.txt and tipOfTheDay.txt from the official European Ragnarok Online (euRO) Client.\r\n\r\n"\
    "Designed and (still being) developed on\r\nMicrosoft Windows 98 Second Edition.\r\n\r\n"\
    "TFdWIFFSVyBEUSBEVlZEVlZMUSwgTFdWIEQgUUxRTUQ7IFFSVyBZRFBTTFVILVJRSCwgV0tSWEpLLg=="

#endif  /* _RESOURCE_H_ */
