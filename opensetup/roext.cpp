// -----------------------------------------------------------------
// RagnarokOnline OpenSetup
// (c) 2010 Ai4rei/AN
// See doc/license.txt for details.
// -----------------------------------------------------------------

#include "snippets/btypes.h"
#include "snippets/cpenum.h"
#include "snippets/cstr.h"

#include <stdlib.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commctrl.h>

#include "resource.h"
#include "tab.h"
#include "ui.h"

#include "roext.h"

#pragma comment(lib,"version.lib")

#define ROEXT_DLLFILE ".\\dinput.dll"
#define ROEXT_INIFILE ".\\dinput.ini"

CROExtSettings::CROExtSettings()
{
    char* lpszSlash;
    unsigned long luLength;

    if((luLength = GetModuleFileName(NULL, this->szIniFile, sizeof(this->szIniFile)))>0)
    {
        for(lpszSlash = this->szIniFile+luLength; lpszSlash>this->szIniFile && lpszSlash[-1]!='\\'; lpszSlash--);
        if(this->szIniFile!=lpszSlash)
        {// full path without executable name, ending with "\\"
            lstrcpy(lpszSlash, ROEXT_INIFILE);
            return;
        }
    }
    DebugBreakHere();
}

CROExtSettings::~CROExtSettings()
{
    WritePrivateProfileString(NULL, NULL, NULL, this->szIniFile);
}

int __stdcall CROExtSettings::Get(enum ROExtSettingEntry nEntry)
{
    switch(nEntry)
    {
        case ROESE_MOUSEFREEDOM:      return this->nMouseFreedom;
        case ROESE_REMAPMOUSEBUTTONS: return this->nRemapMouseButtons;
        case ROESE_REMAPALTF4:        return this->nRemapAltF4;
        case ROESE_KEYREMAP:          return this->nKeyRemap;
        case ROESE_AUTOFREECPU:       return this->nAutoFreeCPU;
        case ROESE_WINDOWONTOP:       return this->nWindowOnTop;
        case ROESE_WINDOWLOCK:        return this->nWindowLock;
        case ROESE_WINDOWPOSX:        return this->nWindowPosX;
        case ROESE_WINDOWPOSY:        return this->nWindowPosY;
        case ROESE_WINDOWWIDTH:       return this->nWindowWidth;
        case ROESE_WINDOWHEIGHT:      return this->nWindowHeight;
        case ROESE_CODEPAGE:          return this->nCodePage;
    }
    DebugBreakHere();
    return 0;
}

void __stdcall CROExtSettings::Set(enum ROExtSettingEntry nEntry, int nValue)
{
    switch(nEntry)
    {
        case ROESE_MOUSEFREEDOM:      this->nMouseFreedom      = nValue; break;
        case ROESE_REMAPMOUSEBUTTONS: this->nRemapMouseButtons = nValue; break;
        case ROESE_REMAPALTF4:        this->nRemapAltF4        = nValue; break;
        case ROESE_KEYREMAP:          this->nKeyRemap          = nValue; break;
        case ROESE_AUTOFREECPU:       this->nAutoFreeCPU       = nValue; break;
        case ROESE_WINDOWONTOP:       this->nWindowOnTop       = nValue; break;
        case ROESE_WINDOWLOCK:        this->nWindowLock        = nValue; break;
        case ROESE_WINDOWPOSX:        this->nWindowPosX        = nValue; break;
        case ROESE_WINDOWPOSY:        this->nWindowPosY        = nValue; break;
        case ROESE_WINDOWWIDTH:       this->nWindowWidth       = nValue; break;
        case ROESE_WINDOWHEIGHT:      this->nWindowHeight      = nValue; break;
        case ROESE_CODEPAGE:          this->nCodePage          = nValue; break;
        default: DebugBreakHere();
    }
}

void __stdcall CROExtSettings::Save(void)
{
    char szBuffer[16];
    unsigned long i;
    struct IniSaveInfo
    {
        const char* lpszKeyName;
        int* lpnValue;
    }
    SaveInfo[] =
    {
#define SAVEENTRY(name) { #name, &this->n##name }
        SAVEENTRY(MouseFreedom      ),
        SAVEENTRY(RemapMouseButtons ),
        SAVEENTRY(RemapAltF4        ),
        SAVEENTRY(KeyRemap          ),
        SAVEENTRY(AutoFreeCPU       ),
        SAVEENTRY(WindowOnTop       ),
        SAVEENTRY(WindowLock        ),
        SAVEENTRY(WindowPosX        ),
        SAVEENTRY(WindowPosY        ),
        SAVEENTRY(WindowWidth       ),
        SAVEENTRY(WindowHeight      ),
        SAVEENTRY(CodePage          ),
#undef SAVEENTRY
    };

    for(i = 0; i<ARRAYLEN(SaveInfo); i++)
    {
        wsprintf(szBuffer, "%d", *(SaveInfo[i].lpnValue));
        WritePrivateProfileString("ROExt", SaveInfo[i].lpszKeyName, szBuffer, this->szIniFile);
    }
}

void __stdcall CROExtSettings::Load(void)
{
    char szBuffer[16];
    unsigned long i;
    struct IniLoadInfo
    {
        const char* lpszKeyName;
        int* lpnValue;
    }
    LoadInfo[] =
    {
#define LOADENTRY(name) { #name, &this->n##name }
        LOADENTRY(MouseFreedom      ),
        LOADENTRY(RemapMouseButtons ),
        LOADENTRY(RemapAltF4        ),
        LOADENTRY(KeyRemap          ),
        LOADENTRY(AutoFreeCPU       ),
        LOADENTRY(WindowOnTop       ),
        LOADENTRY(WindowLock        ),
        LOADENTRY(WindowPosX        ),
        LOADENTRY(WindowPosY        ),
        LOADENTRY(WindowWidth       ),
        LOADENTRY(WindowHeight      ),
        LOADENTRY(CodePage          ),
#undef LOADENTRY
    };

    this->Reset();

    if(GetFileAttributes(this->szIniFile)!=0xffffffff)
    {
        for(i = 0; i<ARRAYLEN(LoadInfo); i++)
        {
            if(GetPrivateProfileString("ROExt", LoadInfo[i].lpszKeyName, "", szBuffer, sizeof(szBuffer), this->szIniFile))
            {
                *(LoadInfo[i].lpnValue) = atoi(szBuffer);
            }
        }
    }
}

void __stdcall CROExtSettings::Reset(void)
{
    this->Set(ROESE_MOUSEFREEDOM,      1 );
    this->Set(ROESE_REMAPMOUSEBUTTONS, 1 );
    this->Set(ROESE_REMAPALTF4,        1 );
    this->Set(ROESE_KEYREMAP,          0 );
    this->Set(ROESE_AUTOFREECPU,       1 );
    this->Set(ROESE_WINDOWONTOP,       0 );
    this->Set(ROESE_WINDOWLOCK,        1 );
    this->Set(ROESE_WINDOWPOSX,        0 );
    this->Set(ROESE_WINDOWPOSY,        0 );
    this->Set(ROESE_WINDOWWIDTH,       0 );
    this->Set(ROESE_WINDOWHEIGHT,      0 );
    this->Set(ROESE_CODEPAGE,         -1 );
};

static BOOL CALLBACK ROExtTabProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch(uMsg)
    {
        case WM_COMMAND:
            switch(HIWORD(wParam))
            {
                case BN_CLICKED:
                    if(LOWORD(wParam)==IDCHECKBOX_MOUSEFREEDOM)
                    {
                        EnableWindow(GetDlgItem(hWnd, IDCHECKBOX_REMAPMOUSEBUTTONS), (BOOL)(IsDlgButtonChecked(hWnd, IDCHECKBOX_MOUSEFREEDOM)==BST_CHECKED));
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

bool __stdcall CROExt::Detect(void)
{
    bool bPresent;
    char szKeyName[64];
    unsigned long i, luVersionInfoSize, luDummy, luLangSize;
    unsigned long* lpluLang;
    void* lpVersionInfo;
    const char* lpszCopyright;

    // By default, it does not exist.
    bPresent = false;

    // How to differentiate ROExt from mouse-freedom and such? Well,
    // the author states, that the ROExt dinput.dll is small (~10k)
    // and it's version resource copyright field contains 'Ruri'. It
    // is not completely fool-proof, but well.
    if((luVersionInfoSize = GetFileVersionInfoSize(ROEXT_DLLFILE, &luDummy))>0)
    {
        if((lpVersionInfo = GlobalAlloc(GMEM_FIXED, luVersionInfoSize))!=NULL)
        {
            if(GetFileVersionInfo(ROEXT_DLLFILE, 0, luVersionInfoSize, lpVersionInfo))
            {
                if(VerQueryValue(lpVersionInfo, "\\VarFileInfo\\Translation", (void**)&lpluLang, (unsigned int*)&luLangSize))
                {
                    for(i = 0; i<luLangSize/sizeof(unsigned long); i++)
                    {
                        wsprintfA(szKeyName, "\\StringFileInfo\\%04x%04x\\LegalCopyright", LOWORD(lpluLang[i]), HIWORD(lpluLang[i]));

                        if(VerQueryValue(lpVersionInfo, szKeyName, (void**)&lpszCopyright, (unsigned int*)&luDummy) && !lstrcmpiA(lpszCopyright, "Ruri"))
                        {
                            bPresent = true;
                            break;
                        }
                    }
                }
            }
            GlobalFree(lpVersionInfo);
        }
    }
    return bPresent;
}

CROExt::CROExt()
{
    if((this->bIsPresent = CROExt::Detect())==true)
    {
        CPE_Init();
        CPE_EnumCodePagesEx(&this->CodePageInfo, CPE_FLAG_INSTALLEDONLY|CPE_FLAG_SORTID);

        this->Settings.Load();
    }
}

CROExt::~CROExt()
{
    if(this->bIsPresent)
    {
        CPE_Quit();
    }
}

void __stdcall CROExt::Load(class CTabMgr* TabMgr)
{
    if(this->bIsPresent)
    {
        this->hWnd = TabMgr->AddTab("ROExt", IDROEXT, IMI_PLUGS, false, MAKEINTRESOURCE(IDD_TAB_ROEXT), &ROExtTabProc);
        this->SetTab();
    }
}

void __stdcall CROExt::Save(void)
{
    if(this->bIsPresent)
    {
        this->GetTab();
        this->Settings.Save();
    }
}

void __stdcall CROExt::Show(int nShowCmd)
{
    if(this->bIsPresent)
    {
        ShowWindow(this->hWnd, nShowCmd);
    }
}

void __stdcall CROExt::GetTab(void)
{
    if(this->bIsPresent)
    {
        int i;

        this->Settings.Set(ROESE_MOUSEFREEDOM,      SendMessage(GetDlgItem(this->hWnd, IDCHECKBOX_MOUSEFREEDOM),      BM_GETCHECK, 0, 0));
        this->Settings.Set(ROESE_REMAPMOUSEBUTTONS, SendMessage(GetDlgItem(this->hWnd, IDCHECKBOX_REMAPMOUSEBUTTONS), BM_GETCHECK, 0, 0));
        this->Settings.Set(ROESE_REMAPALTF4,        SendMessage(GetDlgItem(this->hWnd, IDCHECKBOX_REMAPALTF4),        BM_GETCHECK, 0, 0));
        this->Settings.Set(ROESE_AUTOFREECPU,       SendMessage(GetDlgItem(this->hWnd, IDCHECKBOX_AUTOFREECPU),       BM_GETCHECK, 0, 0));
        this->Settings.Set(ROESE_WINDOWONTOP,       SendMessage(GetDlgItem(this->hWnd, IDCHECKBOX_WINDOWONTOP),       BM_GETCHECK, 0, 0));
        this->Settings.Set(ROESE_WINDOWLOCK,        SendMessage(GetDlgItem(this->hWnd, IDCHECKBOX_WINDOWLOCK),        BM_GETCHECK, 0, 0));

        if((i = SendMessage(GetDlgItem(this->hWnd, IDCOMBOBOX_CODEPAGE), CB_GETCURSEL, 0, 0))!=CB_ERR)
        {
            this->Settings.Set(ROESE_CODEPAGE, this->Idx2CP(i));
        }
    }
}

void __stdcall CROExt::SetTab(void)
{
    HWND hChild;

    if(this->bIsPresent)
    {
        unsigned long i;
        struct UIBatchList lpBatchList[] =
        {
            // Initialize check boxes
            { IDCHECKBOX_MOUSEFREEDOM,      BM_SETCHECK,  (WPARAM)this->Settings.Get(ROESE_MOUSEFREEDOM),           0 },
            { IDCHECKBOX_REMAPMOUSEBUTTONS, BM_SETCHECK,  (WPARAM)this->Settings.Get(ROESE_REMAPMOUSEBUTTONS),      0 },
            { IDCHECKBOX_REMAPALTF4,        BM_SETCHECK,  (WPARAM)this->Settings.Get(ROESE_REMAPALTF4),             0 },
            { IDCHECKBOX_AUTOFREECPU,       BM_SETCHECK,  (WPARAM)this->Settings.Get(ROESE_AUTOFREECPU),            0 },
            { IDCHECKBOX_WINDOWONTOP,       BM_SETCHECK,  (WPARAM)this->Settings.Get(ROESE_WINDOWONTOP),            0 },
            { IDCHECKBOX_WINDOWLOCK,        BM_SETCHECK,  (WPARAM)this->Settings.Get(ROESE_WINDOWLOCK),             0 },
            // Initialize combo boxes
            { IDCOMBOBOX_CODEPAGE,          CB_SETCURSEL, (WPARAM)this->CP2Idx(this->Settings.Get(ROESE_CODEPAGE)), 0 },
        };

        // feed the lists
        hChild = GetDlgItem(this->hWnd, IDCOMBOBOX_CODEPAGE);
        SendMessage(hChild, CB_ADDSTRING, 0, (LPARAM)"(default)");
        for(i = 0; i<this->CodePageInfo.luEntries; i++)
        {
            SendMessage(hChild, CB_ADDSTRING, 0, (LPARAM)this->CodePageInfo.Entries[i].szCodePageName);
        }

        // process simple initializers
        UI::BatchMessage(this->hWnd, lpBatchList, ARRAYLEN(lpBatchList));

        // dependencies
        EnableWindow(GetDlgItem(this->hWnd, IDCHECKBOX_REMAPMOUSEBUTTONS), (BOOL)(IsDlgButtonChecked(this->hWnd, IDCHECKBOX_MOUSEFREEDOM)==BST_CHECKED));
    }
}

int __stdcall CROExt::CP2Idx(int nCodePage)
{
    int i;

    if(nCodePage!=-1)
    {
        for(i = 1; i<=(int)this->CodePageInfo.luEntries; i++)
        {
            if((int)this->CodePageInfo.Entries[i-1].luCodePage==nCodePage)
            {
                return i;
            }
        }
    }
    return 0;
}

int __stdcall CROExt::Idx2CP(int nIdx)
{
    if(0<nIdx && nIdx<=(int)this->CodePageInfo.luEntries)
    {
        return this->CodePageInfo.Entries[nIdx-1].luCodePage;
    }
    return -1;
}
