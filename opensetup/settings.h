// -----------------------------------------------------------------
// RagnarokOnline OpenSetup
// (c) 2010-2013 Ai4rei/AN
// See doc/license.txt for details.
//
// -----------------------------------------------------------------

#ifndef _SETTINGS_H_
#define _SETTINGS_H_

typedef enum SETTINGENGINEID
{
    SENGINE_LUA = 'L'|('U'<<8)|('A'<<16),
    SENGINE_REG = 'R'|('E'<<8)|('G'<<16),
}
SETTINGENGINEID;

typedef enum SETTINGENTRY
{
    SE_ISFULLSCREENMODE = 0,
    SE_WIDTH,
    SE_HEIGHT,
    SE_BITPERPIXEL,
    SE_DEVICECNT,
    SE_MODECNT,
    SE_ISVOODOO,        // REG
    SE_ISLIGHTMAP,
    SE_SPRITEMODE,
    SE_TEXTUREMODE,
    SE_NUMSAMPLETYPE,   // REG
    SE_FOG,
    SE_SOUNDMODE,       // REG, even on LUA with values 0 and 1 only.
    SE_SPEAKERTYPE,     // REG, even on LUA with value 0.
    SE_DIGITALRATETYPE, // REG, even on LUA with value 0.
    SE_DIGITALBITSTYPE, // REG, even on LUA with value 0.
    SE_GUIDDRIVER,      // REG, even on LUA.
    SE_GUIDDEVICE,      // REG, even on LUA.
    SE_DEVICENAME,      // REG, even on LUA.
    SE_PROVIDERNAME,    // REG, even on LUA with value "8—A".
    // added-value
    SE_SHOWTIPSATSTARTUP,
    SE_TRILINEARFILTER,
    // 2.0
    SE_STREAMVOLUME,
    SE_SOUNDVOLUME,
    SE_MOUSEEXCLUSIVE,  // LUA
    SE_BGMISPAUSED,
    SE_ISSOUNDON,
    SE_NOTRADE,         // LUA
    SE_NOSHIFT,         // LUA
    SE_NOCTRL,
    SE_SKILLFAIL,       // LUA
    SE_NOTALKMSG,       // LUA
    SE_NOTALKMSG2,      // LUA
    SE_SHOWNAME,        // LUA
    SE_AURA,            // LUA
    SE_WINDOW,          // LUA
    SE_MAKEMISSEFFECT,
    SE_ISEFFECTON,
    SE_SHOPPING,        // LUA
    SE_STATEINFO,       // LUA
    SE_LOGINOUT,        // LUA
    SE_SNAP,
    SE_ISITEMSNAP,
    SE_ISFIXEDCAMERA,
    SE_ONHOUSERAI,
    SE_ONMERUSERAI,
}
SETTINGENTRY;

typedef struct SETTINGSENTRIES
{
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
    // added-value
    unsigned long SHOWTIPSATSTARTUP;// Whether "Tip of the day" is displayed after logging into the zone (1) or not (0).
    unsigned long TRILINEARFILTER;  // Whether trilinear filtering (smoothing) is on (1) or not (0).
    // added-value end
    // 2.0
    unsigned long STREAMVOLUME;     // BGM volume, ranging from mute (0) to max (127).
    unsigned long SOUNDVOLUME;      // Sound volume, ranging from mute (0) to max (127).
    unsigned long MOUSEEXCLUSIVE;   // Whether mouse movements are restricted to the client window (1) or not (0).
    unsigned long BGMISPAUSED;      // Whether /bgm is turned on (0) or not (1) for REG, turned on (1) or not (0) for LUA.
    unsigned long ISSOUNDON;        // Whether /sound is turned on (1) or not (0).
    unsigned long NOTRADE;          // Whether /notrade is turned on (1) or not (0).
    unsigned long NOSHIFT;          // Whether /noshift is turned on (1) or not (0).
    unsigned long NOCTRL;           // Whether /noctrl is turned on (1) or not (0).
    unsigned long SKILLFAIL;        // Whether /skillfail is turned on (1) or not (0).
    unsigned long NOTALKMSG;        // Whether /notalkmsg is turned on (1) or not (0).
    unsigned long NOTALKMSG2;       // Whether /notalkmsg2 is turned on (1) or not (0).
    unsigned long SHOWNAME;         // Whether /showname is turned on (1) or not (0).
    unsigned long AURA;             // Whether /aura is turned on (1) or not (0).
    unsigned long WINDOW;           // Whether /window is turned on (1) or not (0).
    unsigned long MAKEMISSEFFECT;   // Whether /miss is turned on (1) or not (0).
    unsigned long ISEFFECTON;       // Whether /effect is turned on (1) or not (0).
    unsigned long SHOPPING;         // Whether /shopping is turned on (1) or not (0).
    unsigned long STATEINFO;        // Whether /stateinfo is turned on (1) or not (0).
    unsigned long LOGINOUT;         // Whether /loginout is turned on (1) or not (0).
    unsigned long SNAP;             // Whether /snap is turned on (1) or not (0).
    unsigned long ISITEMSNAP;       // Whether /itemsnap is turned on (1) or not (0).
    unsigned long ISFIXEDCAMERA;    // Whether /camera is turned on (1) or not (0).
    unsigned long ONHOUSERAI;       // Whether /hoai is turned on (1) or not (0).
    unsigned long ONMERUSERAI;      // Whether /merai is turned on (1) or not (0).
    // 2.0 end
    GUID GUIDDRIVER;
    GUID GUIDDEVICE;
    char DEVICENAME[40];            // Driver/Device name selected in the combo box.
    char PROVIDERNAME[1024];        // Appears to be always "No Provider".
}
SETTINGSENTRIES;

class CSettings
{
protected:
    SETTINGSENTRIES m_Entries;

public:
    unsigned long __stdcall Get(SETTINGENTRY nEntry);
    void __stdcall Set(SETTINGENTRY nEntry, unsigned long luValue);
    void __stdcall Set(SETTINGENTRY nEntry, GUID* lpGuid);
    void __stdcall Set(SETTINGENTRY nEntry, const char* lpszString);
    unsigned long __stdcall SaveToIPC(void);
    void __stdcall LoadFromIPC(unsigned long luHash);
    virtual void __stdcall Save(void) = 0;
    virtual void __stdcall Load(void) = 0;
    virtual void __stdcall Reset(void) = 0;
    virtual bool __stdcall IsAvail(SETTINGENTRY nEntry) = 0;
    virtual bool __stdcall IsAdminRequired(void) = 0;
    virtual SETTINGENGINEID __stdcall GetEngineID(void) = 0;
};

#endif  /* _SETTINGS_H_ */
