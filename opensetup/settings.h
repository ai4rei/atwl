// -----------------------------------------------------------------
// RagnarokOnline OpenSetup
// (c) 2010 Ai4rei/AN
// See doc/license.txt for details.
// -----------------------------------------------------------------

#ifndef _SETTINGS_H_
#define _SETTINGS_H_

enum SettingEntry
{
    SE_ISFULLSCREENMODE = 0,
    SE_WIDTH,
    SE_HEIGHT,
    SE_BITPERPIXEL,
    SE_DEVICECNT,
    SE_MODECNT,
    SE_ISVOODOO,
    SE_ISLIGHTMAP,
    SE_SPRITEMODE,
    SE_TEXTUREMODE,
    SE_NUMSAMPLETYPE,
    SE_FOG,
    SE_SOUNDMODE,
    SE_SPEAKERTYPE,
    SE_DIGITALRATETYPE,
    SE_DIGITALBITSTYPE,
    SE_GUIDDRIVER,
    SE_GUIDDEVICE,
    SE_DEVICENAME,
    SE_PROVIDERNAME,
};

class CSettings
{
private:
    unsigned long ISFULLSCREENMODE; // Whether the game is runs in full screen (1) or not (0).
    unsigned long WIDTH;            // Width of the window or full screen in pixel.
    unsigned long HEIGHT;           // Height of the window or full screen in pixel.
    unsigned long BITPERPIXEL;      // Color bit depth used in window or full screen in BPP.
    unsigned long DEVICECNT;        // Driver/Device selection zero-based combo box index.
    unsigned long MODECNT;          // Mode selection zero-based combo box index.
    unsigned long ISVOODOO;         // Whether to use VooDoo3 card hacks (1) or not (0).
    unsigned long ISLIGHTMAP;       // Whether to use light maps (1) or not (0).
    unsigned long SPRITEMODE;       // Sprite quality setting, which can be low (0), middle (1) and high (2).
    unsigned long TEXTUREMODE;      // Texture quality setting, which can be low (0), middle (1) and high (2).
    unsigned long NUMSAMPLETYPE;    // Sound channels, which can be "48 Channel" (0), "32 Channel" (1) or "16 Channel" (2).
    unsigned long FOG;              // Whether fog effect is enabled (1) or not (0).
    unsigned long SOUNDMODE;        // Sound driver type, which can be "No Sound" (0), "Use 2D Sound" (1) or "Use 3D Sound" (2).
    unsigned long SPEAKERTYPE;      // Speaker type, which can be "2 SPEAKER" (0), "HEADPHONE" (1), "SURROUND" (2), "4 SPEAKER" (3)
    unsigned long DIGITALRATETYPE;  // Sound bitrate, which can be "22k" (0), "11k" (1) or "8k" (2).
    unsigned long DIGITALBITSTYPE;  // Sound bitdepth, which can be "16 Bit" (0) or "8 Bit" (1).
    GUID GUIDDRIVER;
    GUID GUIDDEVICE;
    char DEVICENAME[40];            // Driver/Device name selected in the combo box.
    char PROVIDERNAME[1024];        // Appears to be always "No Provider".
public:
    unsigned long __stdcall Get(enum SettingEntry nEntry);
    void __stdcall Set(enum SettingEntry nEntry, unsigned long luValue);
    void __stdcall Set(enum SettingEntry nEntry, GUID* lpGuid);
    void __stdcall Set(enum SettingEntry nEntry, const char* lpszString);
    void __stdcall Save(void);
    void __stdcall Load(void);
    void __stdcall Reset(void);
};

#endif  /* _SETTINGS_H_ */
