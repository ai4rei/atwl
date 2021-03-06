// -----------------------------------------------------------------
// RO Credentials (ROCred)
// (c) 2012+ Ai4rei/AN
//
// -----------------------------------------------------------------

#include <cstdio>

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <tchar.h>
#include <wincred.h>

#include <btypes.h>
#include <bvcstr.h>
#include <dlgabout.h>
#include <dlgtempl.h>
#include <macaddr.h>
#include <md5.h>
#include <memdbg.h>
#include <memtaf.h>
#include <w32dll.h>  /* needed for w32cred */
#include <w32cred.h>
#include <w32ex.h>
#include <w32shell.h>
#include <w32ui.h>
#include <xf_binhex.h>

#include "bgskin.h"
#include "button.h"
#include "config.h"

#include "rocred.h"

static DLGTEMPLATEITEMINFO const l_DlgItems[] =
{
    { DLGTEMPLATEITEM_CLASS_EDIT,   ES_AUTOHSCROLL|WS_BORDER|WS_TABSTOP,                0, IDC_USERNAME,    0, 0, 110, 14, },
    { DLGTEMPLATEITEM_CLASS_EDIT,   ES_AUTOHSCROLL|ES_PASSWORD|WS_BORDER|WS_TABSTOP,    0, IDC_PASSWORD,    0, 0, 110, 14, },
    { DLGTEMPLATEITEM_CLASS_BUTTON, BS_AUTOCHECKBOX|WS_TABSTOP,                         0, IDC_CHECKSAVE,   0, 0, 110, 10, },
};
static DLGTEMPLATEINFO const l_DlgTempl =
{
    L"Tahoma",
    DS_CENTER|DS_SETFONT|WS_POPUPWINDOW|WS_CAPTION,
    0,
    __ARRAYSIZE(l_DlgItems),
    9,
    0,
    0,
    190,
    82,
    l_DlgItems,
};

BEGINENUM(MISCINFOOPTION)
{
    MISCINFO_OPT_MACADDRESS = 0x1,
}
CLOSEENUM(MISCINFOOPTION);
static UINT const l_uMiscInfoOptName[] =
{
    IDS_MISCINFO_OPT_MACADDRESS,
};

// DefDlgEx: DialogProc message cracker system (WindowsX).
static BOOL l_bDefDlgEx = FALSE;
static LRESULT CALLBACK DefWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

int __WDECL MsgBox(HWND const hWnd, LPCSTR const lpszTextOrResource, DWORD const dwFlags)
{
    char szTitle[64];
    char szMessage[4096];
    LPCSTR lpszText;
    HINSTANCE const hInstance = (hWnd!=NULL) ? GetWindowInstance(hWnd) : GetModuleHandle(NULL);

    szTitle[0] = '\0';
    LoadStringA(hInstance, IDS_TITLE, szTitle, __ARRAYSIZE(szTitle));

    if(IS_INTRESOURCE(lpszTextOrResource))
    {// resource string, see MAKEINTRESOURCE on how this works
        szMessage[0] = '\0';
        LoadStringA(hInstance, (UINT)DEGRADE_POINTER(lpszTextOrResource), szMessage, __ARRAYSIZE(szMessage));
        lpszText = szMessage;
    }
    else
    {
        lpszText = lpszTextOrResource;
    }

    return MessageBoxA(hWnd, lpszText, szTitle, dwFlags);
}

bool __WDECL GetFileClassFromExtension(char const* const lpszExtension, char* const lpszBuffer, size_t const uBufferSize)
{
    bool bSuccess = false;

    if(lpszExtension[0]=='.')
    {// it's a file extension
        HKEY hKey;

        if(ERROR_SUCCESS==RegOpenKeyExA(HKEY_CLASSES_ROOT, lpszExtension, 0, KEY_QUERY_VALUE, &hKey))
        {
            DWORD dwBufferBytes = sizeof(lpszBuffer[0])*uBufferSize;

            if(ERROR_SUCCESS==RegQueryValueExA(hKey, NULL, NULL, NULL, (LPBYTE)lpszBuffer, &dwBufferBytes))
            {
                bSuccess = true;
            }

            RegCloseKey(hKey);
        }
    }

    return bSuccess;
}

static bool __WDECL MiscInfoAgreePrompt(HWND const hWnd)
{
    char szBuffer[256*3+128];
    char szPrefix[256], szSuffix[256], szInfo[256] = { 0 };
    int nMiscInfo, nMiscInfoAgree;
    unsigned long luIdx;
    HINSTANCE hInstance = GetWindowInstance(hWnd);

    nMiscInfo = ConfigGetInt("MiscInfo");
    nMiscInfoAgree = ConfigGetInt("_MiscInfoLastAgreed");

    if(nMiscInfo==nMiscInfoAgree)
    {
        return true;
    }

    LoadStringA(hInstance, IDS_MISCINFO_PROMPT_PREFIX, szPrefix, __ARRAYSIZE(szPrefix));
    LoadStringA(hInstance, IDS_MISCINFO_PROMPT_SUFFIX, szSuffix, __ARRAYSIZE(szSuffix));

    for(luIdx = 0; luIdx<__ARRAYSIZE(l_uMiscInfoOptName); luIdx++)
    {
        if(nMiscInfo&(1<<luIdx))
        {
            strcat(szInfo, "\r\n\t- ");
            LoadStringA(hInstance, l_uMiscInfoOptName[luIdx], szInfo+strlen(szInfo), __ARRAYSIZE(szInfo)-strlen(szInfo));
        }
    }

    snprintf(szBuffer, __ARRAYSIZE(szBuffer), "%s%s\r\n\r\n%s", szPrefix, szInfo, szSuffix);

    if(MsgBox(hWnd, szBuffer, MB_YESNO|MB_ICONQUESTION|MB_DEFBUTTON2)==IDYES)
    {
        ConfigSetInt("_MiscInfoLastAgreed", nMiscInfo);
        return true;
    }

    return false;
}

static void __WDECL CombineExePathName(char* const lpszExePath, size_t const uExePathSize, char const* const lpszExeName)
{
    char szFileName[MAX_PATH];

    GetModuleFileNameA(NULL, szFileName, __ARRAYSIZE(szFileName));
    BvStrSplitWindowsSpecificPathComponentsA(szFileName, NULL, NULL);

    snprintf(lpszExePath, uExePathSize, "%s\\%s", szFileName, lpszExeName);
}

// Waits for an process to exit, while keeping the application idle,
// responsive and hidden.
static void __WDECL IdleWaitProcess(HWND const hWnd, HANDLE const hProcess)
{
    bool bTrayIcon = !ConfigGetInt("PolicyNoTrayIcon");
    HINSTANCE const hInstance = GetWindowInstance(hWnd);
    NOTIFYICONDATA_V1A Nid = { sizeof(Nid) };

    ShowWindow(hWnd, SW_MINIMIZE);
    Sleep(200);  // animation
    ShowWindow(hWnd, SW_HIDE);

    if(bTrayIcon)
    {
        // set up notification icon (no 'auto restore' or 'load later')
        Nid.hWnd   = hWnd;
        Nid.uID    = 1;
        Nid.uFlags = NIF_ICON|NIF_TIP;
        Nid.hIcon = LoadSmallIconA(hInstance, MAKEINTRESOURCEA(IDI_MAINICON));
        LoadStringA(hInstance, IDS_TITLE, Nid.szTip, __ARRAYSIZE(Nid.szTip));
        Shell_NotifyIconA(NIM_ADD, (NOTIFYICONDATAA*)&Nid);
    }

    // go idle
    SetPriorityClass(GetCurrentProcess(), IDLE_PRIORITY_CLASS);

    // stay aware and wait for process to exit
    while(MsgWaitForMultipleObjects(1, &hProcess, FALSE, INFINITE, QS_ALLINPUT)!=WAIT_OBJECT_0)
    {
        MSG Msg;

        while(PeekMessage(&Msg, NULL, 0, 0, PM_REMOVE))
        {
            if(!IsDialogMessage(hWnd, &Msg))
            {
                TranslateMessage(&Msg);
                DispatchMessage(&Msg);
            }
        }
    }

    // return to normal
    SetPriorityClass(GetCurrentProcess(), NORMAL_PRIORITY_CLASS);

    if(bTrayIcon)
    {
        Shell_NotifyIconA(NIM_DELETE, (NOTIFYICONDATAA*)&Nid);
    }

    ShowWindow(hWnd, SW_SHOWMINIMIZED);
    ShowWindow(hWnd, SW_RESTORE);
}

// Run and execute process.
static bool __CDECL InvokeProcess(HWND const hWnd, char const* const lpszApplication, char const* const lpszParamFmt, ...)
{
    bool bSuccess = false;
    char szBuffer[1024], szFileClass[MAX_REGISTRY_KEY_SIZE+1];
    va_list lpVl;
    SHELLEXECUTEINFOA Sei = { sizeof(Sei) };

    va_start(lpVl, lpszParamFmt);
    wvsprintfA(szBuffer, lpszParamFmt, lpVl);
    va_end(lpVl);

    Sei.fMask = SEE_MASK_NOCLOSEPROCESS|SEE_MASK_FLAG_NO_UI|(ISUNCPATH(lpszApplication) ? SEE_MASK_CONNECTNETDRV : 0)|SEE_MASK_CLASSNAME;
    Sei.hwnd = hWnd;
    Sei.lpVerb = NULL;
    Sei.lpFile = lpszApplication;
    Sei.lpParameters = szBuffer;
    Sei.nShow = SW_SHOWNORMAL;

    // consider the file an executable, whether or not it looks like it
    if(GetFileClassFromExtension(".exe", szFileClass, __ARRAYSIZE(szFileClass)))
    {
        Sei.lpClass = szFileClass;
    }
    else
    {// should not happen, fallback to default
        Sei.lpClass = "exefile";
    }

    if(ShellExecuteExA(&Sei))
    {
        if(Sei.hProcess)
        {
            IdleWaitProcess(hWnd, Sei.hProcess);
            CloseHandle(Sei.hProcess);

            bSuccess = true;
        }
    }
    else if(GetLastError()==ERROR_CANCELLED)
    {
        // Purpose: The user explicitly canceled a shell UI, such
        // as "Find application to run this file" or UAC prompt,
        // preceding the actual process invocation. As per UX
        // guidelines, such action should not result in a follow up
        // error message.
        ;
    }
    else
    {
        char szMessage[4096];
        char szFormat[256];
        char szError[1024];

        FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, 0, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), szError, __ARRAYSIZE(szError), NULL);
        LoadStringA(GetWindowInstance(hWnd), IDS_EXE_ERROR, szFormat, __ARRAYSIZE(szFormat));
        snprintf(szMessage, __ARRAYSIZE(szMessage), szFormat, lpszApplication, szBuffer, szError);
        MsgBox(hWnd, szMessage, MB_ICONSTOP|MB_OK);

        ZeroMemory((void*)(volatile void*)szMessage, sizeof(szMessage));
    }

    ZeroMemory((void*)(volatile void*)szBuffer, sizeof(szBuffer));

    return bSuccess;
}

static bool __WDECL AppMutexAcquire(HANDLE* const lphMutex)
{
    char szFileName[MAX_PATH];
    char szMutexName[MAX_PATH];

    // care about secondary instances?
    if(ConfigGetInt("SecondInstance"))
    {
        return true;
    }

    // fetch our name (block double execution of this very same exe only)
    GetModuleFileNameA(NULL, szFileName, __ARRAYSIZE(szFileName));

    if(!CreateSingleInstanceA(CreateMsEscapedObjectNameA(szFileName, false, szMutexName, __ARRAYSIZE(szMutexName)), NULL, lphMutex))
    {
        return false;
    }

    return true;
}

static void __WDECL AppMutexRelease(HANDLE* const lphMutex)
{
    if(lphMutex[0]!=NULL)
    {
        DeleteSingleInstance(lphMutex);
    }
}

static bool __WDECL CreateCustomButton(char const* const lpszSection, void* lpContext)
{
    char const* const lpszName = lpszSection+15;  // skip "ROCred.Buttons."
    HWND hWnd = (HWND)lpContext;

    if(ButtonCheckName(lpszName))
    {
        char szBufferDisplayName[4096], szBufferActionData[4096];  // STRINGTABLE limit, which is probably more than you will ever need
        char const* lpszDisplayName;
        char const* lpszActionData;
        char const* lpszActionHandler;
        int nActionType;
        RECT rcBtn;

        rcBtn.left   = ConfigGetIntFromSection(lpszSection, "X");
        rcBtn.top    = ConfigGetIntFromSection(lpszSection, "Y");
        rcBtn.right  = ConfigGetIntFromSection(lpszSection, "W");
        rcBtn.bottom = ConfigGetIntFromSection(lpszSection, "H");
        nActionType  = ConfigGetIntFromSection(lpszSection, "ActionType");
        lpszDisplayName   = ConfigGetStrFromSection(lpszSection, "DisplayName");
        lpszActionData    = ConfigGetStrFromSection(lpszSection, "ActionData");
        lpszActionHandler = ConfigGetStrFromSection(lpszSection, "ActionHandler");

        if(lpszDisplayName[0]=='#')
        {
            LoadStringA(GetWindowInstance(hWnd), BvStrToULongA(lpszDisplayName+1, NULL, 10), szBufferDisplayName, __ARRAYSIZE(szBufferDisplayName));
            lpszDisplayName = szBufferDisplayName;
        }

        if(lpszActionData[0]=='#')
        {
            LoadStringA(GetWindowInstance(hWnd), BvStrToULongA(lpszActionData+1, NULL, 10), szBufferActionData, __ARRAYSIZE(szBufferActionData));
            lpszActionData = szBufferActionData;
        }

        if(ButtonCreate(hWnd, rcBtn.left, rcBtn.top, rcBtn.right /* width */, rcBtn.bottom /* height */, lpszDisplayName, lpszName, nActionType, lpszActionData, lpszActionHandler))
        {
            SetWindowFont(GetDlgItem(hWnd, ButtonGetId(lpszName)), GetWindowFont(hWnd), TRUE);
        }
        else
        {
            MessageBoxA(hWnd, "Failed to create button.", lpszName, MB_ICONSTOP|MB_OK);
        }
    }
    else
    {
        MessageBoxA(hWnd, "Invalid button identifier.", lpszSection, MB_ICONSTOP|MB_OK);
    }

    return true;  // next
}

bool __WDECL StartClient(HWND const hWnd, char const* const lpszExecutable, char const* const lpszParameters)
{
    bool bSuccess = false;
    char szUserName[24];
    char szPassWord[24];
    char szExePath[MAX_PATH];
    char szMiscInfo[128] = { 0 };
    char szTargetName[128];
    char const* lpszExeName;
    char const* lpszExeType;
    BOOL bCheckSave;
    HINSTANCE hInstance = GetWindowInstance(hWnd);

    GetWindowTextA(GetDlgItem(hWnd, IDC_USERNAME), szUserName, __ARRAYSIZE(szUserName));

    if(strlen(szUserName)<4)
    {
        MsgBox(hWnd, szUserName[0] ? MAKEINTRESOURCEA(IDS_USER_SHRT) : MAKEINTRESOURCEA(IDS_USER_NONE), MB_OK|MB_ICONINFORMATION);
        return false;
    }

    GetWindowTextA(GetDlgItem(hWnd, IDC_PASSWORD), szPassWord, __ARRAYSIZE(szPassWord));

    if(strlen(szPassWord)<4)
    {
        MsgBox(hWnd, szPassWord[0] ? MAKEINTRESOURCEA(IDS_PASS_SHRT) : MAKEINTRESOURCEA(IDS_PASS_NONE), MB_OK|MB_ICONINFORMATION);
        return false;
    }

    if(ConfigGetInt("PolicyNoSessionPassword"))
    {
        SetWindowTextA(GetDlgItem(hWnd, IDC_PASSWORD), "");
    }

    bCheckSave = (BOOL)(SendMessage(GetDlgItem(hWnd, IDC_CHECKSAVE), BM_GETCHECK, 0, 0)==BST_CHECKED);
    ConfigSetStr("CheckSave", bCheckSave ? "1" : "0");

    szTargetName[0] = 0;
    if(ConfigGetInt("CheckSavePassword"))
    {
        char const* lpszConfigID = ConfigGetStr("ConfigID");

        if(lpszConfigID[0])
        {
            snprintf(szTargetName, __ARRAYSIZE(szTargetName), "%s%s", ROCRED_TARGET_NAME, lpszConfigID);
        }
    }

    if(bCheckSave)
    {// save
        ConfigSetStr("UserName", szUserName);

        if(szTargetName[0])
        {
            WinCredSaveA(szTargetName, szUserName, szPassWord, CRED_PERSIST_LOCAL_MACHINE);
        }
    }
    else
    {// delete
        ConfigSetStr("UserName", NULL);

        if(szTargetName[0])
        {
            WinCredDeleteA(szTargetName, CRED_TYPE_GENERIC, 0);
        }
    }

    lpszExeName = lpszExecutable;
    lpszExeType = lpszParameters;
    CombineExePathName(szExePath, __ARRAYSIZE(szExePath), lpszExeName);

    // miscellaneous information for the server
    if(ConfigGetInt("MiscInfo"))
    {
        if(MiscInfoAgreePrompt(hWnd))
        {// agreed
            int nInfo = ConfigGetInt("MiscInfo");

            if(nInfo&MISCINFO_OPT_MACADDRESS)
            {
                MACADDRESS Mac;
                MacAddressGet(&Mac, MACADDR_OPT_DEFAULT_ZERO);

                snprintf(szMiscInfo+strlen(szMiscInfo), __ARRAYSIZE(szMiscInfo)-strlen(szMiscInfo), "mac=%02x%02x%02x%02x%02x%02x&", Mac.Address[0], Mac.Address[1], Mac.Address[2], Mac.Address[3], Mac.Address[4], Mac.Address[5]);
            }

            strcat(szMiscInfo, "key=");
        }
        else
        {// did not agree
            return false;
        }
    }

    if(ConfigGetInt("HashMD5"))
    {// MD5
        MD5HASH Hash;
        char szHexHash[sizeof(Hash)*2+1];

        MD5_String(szPassWord, &Hash);
        XF_BinHexA(szHexHash, __ARRAYSIZE(szHexHash), Hash.ucData, sizeof(Hash.ucData));

        bSuccess = InvokeProcess(hWnd, szExePath, "-t:%s%s %s %s", szMiscInfo, szHexHash, szUserName, lpszExeType);
    }
    else
    {// Plaintext
        bSuccess = InvokeProcess(hWnd, szExePath, "-t:%s%s %s %s", szMiscInfo, szPassWord, szUserName, lpszExeType);
    }

    // get rid of the password after it has been used
    ZeroMemory((void*)(volatile void*)szPassWord, sizeof(szPassWord));

    return bSuccess;
}

static BOOL CALLBACK WndProcOnInitDialog(HWND const hWnd, HWND const hWndFocus, LPARAM const lParam)
{
    char szBuffer[4096];
    BOOL bCheckSave, bSetFocus = TRUE;
    HINSTANCE hInstance = GetWindowInstance(hWnd);

    SetWindowLargeIcon(hWnd, LoadLargeIconA(hInstance, MAKEINTRESOURCEA(IDI_MAINICON)));
    SetWindowSmallIcon(hWnd, LoadSmallIconA(hInstance, MAKEINTRESOURCEA(IDI_MAINICON)));

    LoadStringA(hInstance, IDS_TITLE, szBuffer, __ARRAYSIZE(szBuffer));
    SetWindowTextA(hWnd, szBuffer);

    if(ConfigGetInt("PolicyNoCheckSave"))
    {
        ShowWindow(GetDlgItem(hWnd, IDC_CHECKSAVE), SW_HIDE);
    }
    else
    {
        LoadStringA(hInstance, IDS_CHECKSAVE, szBuffer, __ARRAYSIZE(szBuffer));
        SetWindowTextA(GetDlgItem(hWnd, IDC_CHECKSAVE), szBuffer);

        bCheckSave = ConfigGetInt("CheckSave");
        SendMessage(GetDlgItem(hWnd, IDC_CHECKSAVE), BM_SETCHECK, (WPARAM)bCheckSave, 0);

        if(bCheckSave)
        {
            char const* lpszUserName = ConfigGetStr("UserName");

            if(lpszUserName[0])
            {
                SetWindowTextA(GetDlgItem(hWnd, IDC_USERNAME), lpszUserName);
                SetDialogFocus(hWnd, GetDlgItem(hWnd, IDC_PASSWORD));  // move focus to password
                bSetFocus = FALSE;
            }

            if(ConfigGetInt("CheckSavePassword"))
            {// check credential manager
                char const* lpszConfigID = ConfigGetStr("ConfigID");

                if(lpszConfigID[0])
                {
                    char szUserName[24];
                    char szPassWord[24];
                    char szTargetName[128];

                    snprintf(szTargetName, __ARRAYSIZE(szTargetName), "%s%s", ROCRED_TARGET_NAME, lpszConfigID);

                    if(WinCredLoadA(szTargetName, szUserName, __ARRAYSIZE(szUserName), szPassWord, __ARRAYSIZE(szPassWord)))
                    {
                        SetWindowTextA(GetDlgItem(hWnd, IDC_PASSWORD), szPassWord);
                        ZeroMemory((void*)(volatile void*)szPassWord, sizeof(szPassWord));

                        SetWindowTextA(GetDlgItem(hWnd, IDC_USERNAME), szUserName);
                        SetDialogFocus(hWnd, ButtonGetDefault());
                        bSetFocus = FALSE;
                    }
                }
            }
        }
    }

    // load buttons if any
    ConfigForEachSectionMatch("ROCred.Buttons.", &CreateCustomButton, hWnd);

    // apply available skins
    BgSkinInit(hWnd);

    return bSetFocus;
}

static void CALLBACK WndProcOnCommand(HWND const hWnd, int const nId, HWND const hWndCtl, UINT const uCodeNotify)
{
    if(uCodeNotify==0U || uCodeNotify==1U)
    {
        ButtonAction(hWnd, nId);
    }
}

static BOOL CALLBACK WndProcOnEraseBkgnd(HWND const hWnd, HDC const hDC)
{
    if(!BgSkinOnEraseBkGnd(hWnd, hDC))
    {
        return FORWARD_WM_ERASEBKGND(hWnd, hDC, DefWndProc);  // default background
    }

    return TRUE;
}

static void CALLBACK WndProcOnLButtonDown(HWND const hWnd, BOOL const bDoubleClick, int const nX, int const nY, UINT const uKeyFlags)
{
    if(!BgSkinOnLButtonDown(hWnd))
    {
        FORWARD_WM_LBUTTONDOWN(hWnd, bDoubleClick, nX, nY, uKeyFlags, DefWndProc);
    }
}

static HBRUSH CALLBACK WndProcOnCtlColorStatic(HWND const hWnd, HDC const hDC, HWND const hWndChild, int const nType)
{
    HBRUSH hbrBackground = BgSkinOnCtlColorStatic(hDC, hWndChild);

    if(hbrBackground==NULL)
    {
        hbrBackground = FORWARD_WM_CTLCOLORSTATIC(hWnd, hDC, hWndChild, DefWndProc);
    }

    return hbrBackground;
}

static HBRUSH CALLBACK WndProcOnCtlColorEdit(HWND const hWnd, HDC const hDC, HWND const hWndChild, int const nType)
{
    HBRUSH hbrBackground = BgSkinOnCtlColorEdit(hDC, hWndChild);

    if(hbrBackground==NULL)
    {
        hbrBackground = FORWARD_WM_CTLCOLOREDIT(hWnd, hDC, hWndChild, DefWndProc);
    }

    return hbrBackground;
}

static void CALLBACK WndProcOnDrawItem(HWND const hWnd, DRAWITEMSTRUCT const* const lpDrawItem)
{
    if(!BgSkinOnDrawItem(lpDrawItem->CtlID, lpDrawItem))
    {
        FORWARD_WM_DRAWITEM(hWnd, lpDrawItem, DefWndProc);
    }
}

static void CALLBACK WndProcOnHelp(HWND const hWnd, LPHELPINFO const lphi)
{
    DLGABOUTINFO Dai =
    {
        hWnd,
        MAKELONG(2012,2021),
        _T("About ROCred..."),
        _T("ROCred"),
        _T(APP_VERSION),
        _T("Ai4rei/AN"),
        _T("\r\nThis software is FREEWARE and is provided AS IS, without warranty of ANY KIND, either expressed or implied, including but not limited to the implied warranties of merchantability and/or fitness for a particular purpose. If your country's law does not allow complete exclusion of liability, you may not use this software. The author SHALL NOT be held liable for ANY damage to you, your hardware, your software, your pets, your dear other, or to anyone or anything else, that may or may not result from the use or misuse of this software. Basically, you use it at YOUR OWN RISK."),
        _T("http://ai4rei.net/p/rocredweb"),
    };

    DlgAbout(&Dai);
}

static void CALLBACK WndProcOnDestroy(HWND const hWnd)
{
    BgSkinFree();
}

static LRESULT CALLBACK DefWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    return DefDlgProcEx(hWnd, uMsg, wParam, lParam, &l_bDefDlgEx);
}

static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch(uMsg)
    {
    HANDLE_MSG(hWnd, WM_INITDIALOG,     WndProcOnInitDialog);
    HANDLE_MSG(hWnd, WM_COMMAND,        WndProcOnCommand);
    HANDLE_MSG(hWnd, WM_ERASEBKGND,     WndProcOnEraseBkgnd);
    HANDLE_MSG(hWnd, WM_LBUTTONDOWN,    WndProcOnLButtonDown);
    HANDLE_MSG(hWnd, WM_CTLCOLORSTATIC, WndProcOnCtlColorStatic);
    HANDLE_MSG(hWnd, WM_CTLCOLOREDIT,   WndProcOnCtlColorEdit);
    HANDLE_MSG(hWnd, WM_DRAWITEM,       WndProcOnDrawItem);
    HANDLE_MSG(hWnd, WM_HELP,           WndProcOnHelp);
    HANDLE_MSG(hWnd, WM_DESTROY,        WndProcOnDestroy);
    }

    return DefWndProc(hWnd, uMsg, wParam, lParam);
}

static BOOL CALLBACK DlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CheckDefDlgRecursion(&l_bDefDlgEx);

    return SetDlgMsgResult(hWnd, uMsg, WndProc(hWnd, uMsg, wParam, lParam));
}

static MEM_OUTOFMEMORY_ACTION __WDECL OnOOM(LPCMEMOUTOFMEMORYINFO const lpInfo, LPCMEMSTATISTICS const lpStats, void* lpContext)
{// consider the following: we have no memory
    char szMessage[128];

    snprintf(szMessage, __ARRAYSIZE(szMessage), "Failed to allocate %u+%u bytes of memory.", lpInfo->uWantAvail, lpInfo->uWantBytes-lpInfo->uWantAvail);

    if(MsgBox(NULL, szMessage, MB_RETRYCANCEL|MB_ICONSTOP|MB_SYSTEMMODAL)==IDRETRY)
    {
        return MEM_OOM_RETRY;
    }

    return MEM_OOM_DEFAULT;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int nShowCmd)
{
    // memory
    MemSetOptions(MEM_OPT_EXCEPTIO|MEM_OPT_STOPBUGS);
    MemSetHandler(&OnOOM, NULL);

    // start up
    if(!FAILED(CoInitialize(NULL)))
    {
        if(ConfigInit())
        {
            HANDLE hMutex = NULL;

            if(AppMutexAcquire(&hMutex))
            {
                DLGTEMPLATEINFO DlgTi = l_DlgTempl;

                DlgTi.wFontSize = ConfigGetInt("FontSize");

                InitCommonControls();
                DlgTemplateExBoxParam(hInstance, &DlgTi, NULL, &DlgProc, 0);
                AppMutexRelease(&hMutex);
            }

            ConfigQuit();
        }
        else
        {
            MsgBox(NULL, MAKEINTRESOURCEA(IDS_CONFIG_ERROR), MB_OK|MB_ICONSTOP);
        }

        CoUninitialize();
    }
    else
    {
        MsgBox(NULL, MAKEINTRESOURCEA(IDS_COINIT_ERROR), MB_OK|MB_ICONSTOP);
    }

    // memory again
    if(MemIsLeaked() || MemDbgIsBadStats())
    {
        MemDbgPrintStats();
    }

    return 0;
}
