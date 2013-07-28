// -----------------------------------------------------------------
// RO Credentials (ROCred)
// (c) 2012-2013 Ai4rei/AN
//
// -----------------------------------------------------------------

#include <windows.h>
#include <commctrl.h>

#include <btypes.h>
#include <dlgabout.h>
#include <dlgtempl.h>
#include <md5.h>
#include <xf_binhex.h>

#include "config.h"
#include "rocred.h"

static const DLGTEMPLATEITEMINFO l_DlgItems[] =
{
    { DLGTEMPLATEITEM_CLASS_STATIC, 0,                                                  0, IDS_USERNAME,    7,      10, 60,     8,  },
    { DLGTEMPLATEITEM_CLASS_EDIT,   ES_AUTOHSCROLL|WS_BORDER|WS_TABSTOP,                0, IDC_USERNAME,    73,     7,  110,    14, },
    { DLGTEMPLATEITEM_CLASS_STATIC, 0,                                                  0, IDS_PASSWORD,    7,      28, 60,     8,  },
    { DLGTEMPLATEITEM_CLASS_EDIT,   ES_AUTOHSCROLL|ES_PASSWORD|WS_BORDER|WS_TABSTOP,    0, IDC_PASSWORD,    73,     25, 110,    14, },
    { DLGTEMPLATEITEM_CLASS_BUTTON, BS_AUTOCHECKBOX|WS_TABSTOP,                         0, IDC_CHECKSAVE,   73,     43, 110,    10, },
    { DLGTEMPLATEITEM_CLASS_BUTTON, BS_DEFPUSHBUTTON|WS_TABSTOP,                        0, IDOK,            79,     61, 50,     14, },
    { DLGTEMPLATEITEM_CLASS_BUTTON, BS_PUSHBUTTON|WS_TABSTOP,                           0, IDCANCEL,        133,    61, 50,     14, },
    { DLGTEMPLATEITEM_CLASS_BUTTON, BS_PUSHBUTTON|WS_TABSTOP,                           0, IDB_REPLAY,      7,      61, 50,     14, },
};
static const DLGTEMPLATEINFO l_DlgTempl =
{
    L"Tahoma",
    DS_MODALFRAME|DS_CENTER|DS_SETFONT|WS_POPUPWINDOW|WS_CAPTION,
    0,
    __ARRAYSIZE(l_DlgItems),
    9,
    0,
    0,
    190,
    82,
    l_DlgItems,
};

static char l_szAppTitle[1024] = { 0 };

// Waits for an process to exit, while keeping the application idle,
// responsive and hidden.
static void __stdcall IdleWaitProcess(HWND hWnd, HANDLE hProcess)
{
    bool bTrayIcon = !ConfigGetInt("PolicyNoTrayIcon");
    HINSTANCE hInstance = GetModuleHandle(NULL);
    NOTIFYICONDATA Nid = { sizeof(Nid), hWnd, 1, NIF_ICON|NIF_TIP };

    ShowWindow(hWnd, SW_MINIMIZE);
    Sleep(200);  // animation
    ShowWindow(hWnd, SW_HIDE);

    if(bTrayIcon)
    {
        // set up notification icon (no 'auto restore' or 'load later')
        Nid.hIcon = LoadImage(hInstance, MAKEINTRESOURCE(2), IMAGE_ICON, 16, 16, LR_SHARED);
        LoadStringA(hInstance, IDS_TITLE, Nid.szTip, __ARRAYSIZE(Nid.szTip));
        Shell_NotifyIcon(NIM_ADD, &Nid);
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
        Shell_NotifyIcon(NIM_DELETE, &Nid);
    }

    ShowWindow(hWnd, SW_SHOWMINIMIZED);
    ShowWindow(hWnd, SW_RESTORE);
}

// Run and execute process.
static void __cdecl InvokeProcess(HWND hWnd, const char* lpszApplication, const char* lpszParamFmt, ...)
{
    char szBuffer[4096];
    va_list lpVl;
    SHELLEXECUTEINFO Sei = { sizeof(Sei) };

    va_start(lpVl, lpszParamFmt);
    wvsprintfA(szBuffer, lpszParamFmt, lpVl);
    va_end(lpVl);

    Sei.fMask = SEE_MASK_NOCLOSEPROCESS|(ISUNCPATH(lpszApplication) ? SEE_MASK_CONNECTNETDRV : 0);
    Sei.hwnd = hWnd;
    Sei.lpVerb = "open";
    Sei.lpFile = lpszApplication;
    Sei.lpParameters = szBuffer;
    Sei.nShow = SW_SHOWNORMAL;

    if(ShellExecuteEx(&Sei))
    {
        IdleWaitProcess(hWnd, Sei.hProcess);
        CloseHandle(Sei.hProcess);
    }
    else
    {
        // FIXME: improve this
        DWORD dwLastError = GetLastError();
        MessageBox(hWnd, szBuffer, l_szAppTitle, MB_OK|MB_ICONSTOP);
        FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 0, dwLastError, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), szBuffer, __ARRAYSIZE(szBuffer), NULL);
        MessageBox(hWnd, szBuffer, l_szAppTitle, MB_OK|MB_ICONSTOP);
        LoadStringA(GetModuleHandle(NULL), IDS_EXE_ERROR, szBuffer, __ARRAYSIZE(szBuffer));
        MessageBox(hWnd, szBuffer, l_szAppTitle, MB_OK|MB_ICONSTOP);
    }
}

static bool __stdcall AppMutexAcquire(HANDLE* lphMutex)
{
    char* lpszPtr;
    char szMutexName[MAX_PATH];

    // care about secondary instances?
    if(ConfigGetInt("SecondInstance"))
    {
        return true;
    }

    // fetch our name (block double execution of this very same exe only)
    GetModuleFileNameA(NULL, szMutexName, __ARRAYSIZE(szMutexName));

    // replace backslashes with a valid character
    for(lpszPtr = szMutexName; lpszPtr[0] = lpszPtr[0]=='\\' ? '!' : lpszPtr[0]; lpszPtr++);

    if((lphMutex[0] = CreateMutex(NULL, FALSE, szMutexName))==NULL)
    {
        return false;
    }
    else if(GetLastError()==ERROR_ALREADY_EXISTS || WaitForSingleObject(lphMutex[0], 0)!=WAIT_OBJECT_0)
    {
        CloseHandle(lphMutex[0]);
        return false;
    }

    return true;
}

static void __stdcall AppMutexRelease(HANDLE* lphMutex)
{
    if(lphMutex[0])
    {
        CloseHandle(lphMutex[0]);
        lphMutex[0] = NULL;
    }
}

static BOOL CALLBACK DlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch(uMsg)
    {
        case WM_INITDIALOG:
            {
                char szBuffer[4096];
                BOOL bCheckSave, bSetFocus = TRUE;
                HINSTANCE hInstance = GetModuleHandleA(NULL);

                SendMessage(hWnd, WM_SETICON, ICON_BIG,
                    (LPARAM)LoadImage(hInstance, MAKEINTRESOURCE(1), IMAGE_ICON, 32, 32, LR_SHARED));
                SendMessage(hWnd, WM_SETICON, ICON_SMALL,
                    (LPARAM)LoadImage(hInstance, MAKEINTRESOURCE(2), IMAGE_ICON, 16, 16, LR_SHARED));

                SetWindowTextA(hWnd, l_szAppTitle);

                LoadStringA(hInstance, IDS_USERNAME, szBuffer, __ARRAYSIZE(szBuffer));
                SetWindowTextA(GetDlgItem(hWnd, IDS_USERNAME), szBuffer);

                LoadStringA(hInstance, IDS_PASSWORD, szBuffer, __ARRAYSIZE(szBuffer));
                SetWindowTextA(GetDlgItem(hWnd, IDS_PASSWORD), szBuffer);

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
                        ConfigGetStr("UserName", szBuffer, __ARRAYSIZE(szBuffer));

                        if(szBuffer[0])
                        {
                            SetWindowTextA(GetDlgItem(hWnd, IDC_USERNAME), szBuffer);
                            SetFocus(GetDlgItem(hWnd, IDC_PASSWORD));  // move focus to password
                            bSetFocus = FALSE;
                        }
                    }
                }

                LoadStringA(hInstance, IDS_OK, szBuffer, __ARRAYSIZE(szBuffer));
                SetWindowTextA(GetDlgItem(hWnd, IDOK), szBuffer);

                LoadStringA(hInstance, IDS_CLOSE, szBuffer, __ARRAYSIZE(szBuffer));
                SetWindowTextA(GetDlgItem(hWnd, IDCANCEL), szBuffer);

                if(ConfigGetInt("PolicyNoReplay"))
                {
                    ShowWindow(GetDlgItem(hWnd, IDB_REPLAY), SW_HIDE);
                }
                else
                {
                    LoadStringA(hInstance, IDS_REPLAY, szBuffer, __ARRAYSIZE(szBuffer));
                    SetWindowTextA(GetDlgItem(hWnd, IDB_REPLAY), szBuffer);
                }

                return bSetFocus;
            }
        case WM_COMMAND:
            if(HIWORD(wParam)!=1  && HIWORD(wParam)!=0)
            {
                ;
            }
            else switch(LOWORD(wParam))
            {
                case IDB_REPLAY:
                    {
                        char szExePath[MAX_PATH];
                        char szExeName[MAX_PATH];
                        char szExeType[16];

                        ConfigGetStr("ExeName", szExeName, __ARRAYSIZE(szExeName));
                        ConfigGetStr("ExeType", szExeType, __ARRAYSIZE(szExeType));
                        {
                            char* lpszSlash = szExePath+GetModuleFileNameA(NULL, szExePath, __ARRAYSIZE(szExePath));

                            for(; lpszSlash[-1]!='\\'; lpszSlash--);
                            lpszSlash[0] = 0;
                            lstrcat(szExePath, szExeName);
                        }

                        // Replay mode
                        InvokeProcess(hWnd, szExePath, "-t:Replay %s", szExeType);
                    }
                    //EndDialog(hWnd, 1);
                    break;
                case IDOK:
                    {
                        char szUserName[24];
                        char szPassWord[24];
                        char szExePath[MAX_PATH];
                        char szExeName[MAX_PATH];
                        char szExeType[16];
                        char szBuffer[4096];
                        BOOL bCheckSave;
                        HINSTANCE hInstance = GetModuleHandleA(NULL);

                        GetWindowTextA(GetDlgItem(hWnd, IDC_USERNAME), szUserName, __ARRAYSIZE(szUserName));

                        if(lstrlenA(szUserName)<4)
                        {
                            LoadStringA(hInstance, szUserName[0] ? IDS_USER_SHRT : IDS_USER_NONE, szBuffer, __ARRAYSIZE(szBuffer));
                            MessageBox(hWnd, szBuffer, "", MB_OK|MB_ICONINFORMATION);
                            break;
                        }

                        GetWindowTextA(GetDlgItem(hWnd, IDC_PASSWORD), szPassWord, __ARRAYSIZE(szPassWord));

                        if(lstrlenA(szPassWord)<4)
                        {
                            LoadStringA(hInstance, szPassWord[0] ? IDS_PASS_SHRT : IDS_PASS_NONE, szBuffer, __ARRAYSIZE(szBuffer));
                            MessageBox(hWnd, szBuffer, "", MB_OK|MB_ICONINFORMATION);
                            break;
                        }

                        if(ConfigGetInt("PolicyNoSessionPassword"))
                        {
                            SetWindowTextA(GetDlgItem(hWnd, IDC_PASSWORD), "");
                        }

                        bCheckSave = (BOOL)(SendMessage(GetDlgItem(hWnd, 103), BM_GETCHECK, 0, 0)==BST_CHECKED);
                        ConfigSetStr("CheckSave", bCheckSave ? "1" : "0");

                        if(bCheckSave)
                        {// save
                            ConfigSetStr("UserName", szUserName);
                        }
                        else
                        {// delete
                            ConfigSetStr("UserName", NULL);
                        }

                        ConfigGetStr("ExeName", szExeName, __ARRAYSIZE(szExeName));
                        ConfigGetStr("ExeType", szExeType, __ARRAYSIZE(szExeType));
                        {
                            char* lpszSlash = szExePath+GetModuleFileNameA(NULL, szExePath, __ARRAYSIZE(szExePath));

                            for(; lpszSlash[-1]!='\\'; lpszSlash--);
                            lpszSlash[0] = 0;
                            lstrcat(szExePath, szExeName);
                        }

                        if(ConfigGetInt("HashMD5"))
                        {// MD5
                            uint8 ucHash[4*4];
                            char szHexHash[4*4*2+1];

                            MD5_String(szPassWord, ucHash);
                            XF_BinHex(szHexHash, __ARRAYSIZE(szHexHash), ucHash, __ARRAYSIZE(ucHash));

                            InvokeProcess(hWnd, szExePath, "-t:%s %s %s", szHexHash, szUserName, szExeType);
                        }
                        else
                        {// Plaintext
                            InvokeProcess(hWnd, szExePath, "-t:%s %s %s", szPassWord, szUserName, szExeType);
                        }
                    }
                    //EndDialog(hWnd, 1);
                    break;
                case IDCANCEL:
                    EndDialog(hWnd, 0);
                    break;
            }
            break;
        case WM_HELP:
            {
                DLGABOUTINFO Dai =
                {
                    hWnd,
                    MAKELONG(2012,2013),
                    "About ROCred...",
                    "ROCred",
                    APP_VERSION,
                    "Ai4rei/AN",
                    "\r\nThis software is FREEWARE and is provided AS IS, without warranty of ANY KIND, either expressed or implied, including but not limited to the implied warranties of merchantability and/or fitness for a particular purpose. If your country's law does not allow complete exclusion of liability, you may not use this software. The author SHALL NOT be held liable for ANY damage to you, your hardware, your software, your pets, your dear other, or to anyone or anything else, that may or may not result from the use or misuse of this software. Basically, you use it at YOUR OWN RISK.",
                };

                DlgAbout(&Dai);
            }
            break;
        default:
            return FALSE;
    }

    return TRUE;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int nShowCmd)
{
    char szErrMsg[256];
    unsigned char ucDlgBuf[264];
    unsigned long luDlgBufSize = sizeof(ucDlgBuf);
    HANDLE hMutex = NULL;

    // global window title
    LoadStringA(hInstance, IDS_TITLE, l_szAppTitle, __ARRAYSIZE(l_szAppTitle));

    // start up
    if(ConfigInit())
    {
        if(AppMutexAcquire(&hMutex))
        {
            if(lpszCmdLine[0]=='/')
            {
                if(!lstrcmpiA(&lpszCmdLine[1], "embed"))
                {
                    if(!ConfigSave())
                    {
                        MessageBox(NULL, "Embedding configuration failed. Make sure you have a configuration set up and do this on Windows NT or later.", l_szAppTitle, MB_OK|MB_ICONSTOP);
                    }
                }
            }
            else
            {
                AssertHere(DlgTemplate(&l_DlgTempl, ucDlgBuf, &luDlgBufSize));
                InitCommonControls();
                DialogBoxIndirectParam(GetModuleHandle(NULL), (LPCDLGTEMPLATE)ucDlgBuf, NULL, &DlgProc, 0);
            }

            AppMutexRelease(&hMutex);
        }

        ConfigQuit();
    }
    else
    {
        LoadStringA(hInstance, IDS_CONFIG_ERROR, szErrMsg, __ARRAYSIZE(szErrMsg));
        MessageBox(NULL, szErrMsg, l_szAppTitle, MB_OK|MB_ICONSTOP);
    }

    return 0;
}
