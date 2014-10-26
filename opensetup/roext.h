// -----------------------------------------------------------------
// RagnarokOnline OpenSetup
// (c) 2010-2014 Ai4rei/AN
// See doc/license.txt for details.
//
// -----------------------------------------------------------------

#ifndef _ROEXT_H_
#define _ROEXT_H_

typedef enum ROEXTSETTINGENTRY
{
    ROESE_MOUSEFREEDOM,
    ROESE_REMAPMOUSEBUTTONS,
    ROESE_REMAPALTF4,
    ROESE_KEYREMAP,
    ROESE_AUTOFREECPU,
    ROESE_WINDOWONTOP,
    ROESE_WINDOWLOCK,
    ROESE_WINDOWPOSX,
    ROESE_WINDOWPOSY,
    ROESE_WINDOWWIDTH,
    ROESE_WINDOWHEIGHT,
    ROESE_CODEPAGE,
}
ROEXTSETTINGENTRY;

class CROExtSettings
{
private:
    int nMouseFreedom;         // Allow mouse cursor to freely leave and enter RO window.
    int nRemapMouseButtons;    // Remap 3-5'th mouse buttons, requires turned on MouseFreedom.
    int nRemapAltF4;           // Remap closing RO from Alt+F4 to Alt+PrintScreen.
    int nKeyRemap;             // Change active skill set by holding Ctrl and Alt keys.
    int nAutoFreeCPU;          // Release CPU when RO window is inactive.
    int nWindowOnTop;          // Make RO window be always on top.
    int nWindowLock;           // Remove RO window borders and lock its position.
    int nWindowPosX;           // Override RO window position and size.
    int nWindowPosY;
    int nWindowWidth;
    int nWindowHeight;
    int nCodePage;             // Override codepage used by client, use -1 for no override.
    char szIniFile[MAX_PATH];

public:
    CROExtSettings();
    ~CROExtSettings();
    int __stdcall Get(ROEXTSETTINGENTRY nEntry);
    void __stdcall Set(ROEXTSETTINGENTRY nEntry, int nValue);
    void __stdcall Save(void);
    void __stdcall Load(void);
    void __stdcall Reset(void);
};

class CROExt
{
private:
    typedef BOOL (WINAPI* LPFNGETCPINFOEX)(UINT, DWORD, LPCPINFOEX);
    LPFNGETCPINFOEX GetCPInfoEx;

    static bool __stdcall Detect(void);

    class CROExtSettings Settings;
    int nCodePages[400];
    unsigned long luCodePageCount;
    bool bIsPresent;
    HWND hWnd;

public:
    CROExt();
    ~CROExt();
    void __stdcall Load(class CTabMgr* TabMgr);
    void __stdcall Save(void);
    void __stdcall Show(int nShowCmd);
    void __stdcall SetTab(void);
    void __stdcall GetTab(void);

    // codepage stuff
    static int __cdecl SortCodePages(const void* lpItemA, const void* lpItemB);
    static BOOL CALLBACK CollectCodePages(char* lpszCodePage);
    void __stdcall GetCodePageName(int nCodePage, char* lpszBuffer, unsigned long luBufferSize);
    unsigned long __stdcall GetIndexFromCodePage(int nCodePage);
    int __stdcall GetCodePageFromIndex(unsigned long luIndex);
};

#endif  /* _ROEXT_H_ */
