// -----------------------------------------------------------------
// RO Credentials (ROCred)
// (c) 2012+ Ai4rei/AN
//
// -----------------------------------------------------------------

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>

#include <btypes.h>
#include <bvcstr.h>
#include <memtaf.h>
#include <xf_slash.h>

#include "config.h"
#include "rocred.h"

#include "button.h"

BEGINENUM(BUTTON_ACTION)
{
    BUTTON_ACTION_SHELLEXEC = 0,
    BUTTON_ACTION_SHELLEXEC_CLOSE,
    BUTTON_ACTION_CLOSE,
    BUTTON_ACTION_MSGBOX,
    BUTTON_ACTION_SHELLEXEC_CLIENT,
    BUTTON_ACTION_MINIMIZE,
    BUTTON_ACTION_MAX
}
CLOSEENUM(BUTTON_ACTION);

BEGINSTRUCT(BUTTON_DATA)
{
    WNDPROC lpfnPrevWndProc;
    char* lpszName;
    char* lpszActionData;
    char* lpszActionHandler;
    int nActionType;
}
CLOSESTRUCT(BUTTON_DATA);

static HWND l_hDefaultBtnWnd = NULL;  /* handle of the default button */

static LRESULT CALLBACK Button_P_SubclassWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CONTEXTCAST(LPBUTTON_DATA,lpBd,GetWindowLongPtr(hWnd, GWLP_USERDATA));

    if(lpBd)
    {
        WNDPROC lpfnPrevWndProc = lpBd->lpfnPrevWndProc;

        if(uMsg==WM_DESTROY || uMsg==WM_NCDESTROY)
        {// cleanup
            if(GetWindowLongPtr(hWnd, GWLP_WNDPROC)==(LONG_PTR)&Button_P_SubclassWndProc)
            {
                SubclassWindow(hWnd, lpfnPrevWndProc);
                SetWindowLongPtr(hWnd, GWLP_USERDATA, 0);

                MemTFree(&lpBd);
            }
        }

        return CallWindowProc(lpfnPrevWndProc, hWnd, uMsg, wParam, lParam);
    }

    // should not happen
    DebugBreakHere();

    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

bool __WDECL ButtonCreate(HWND const hWndParent, int const nX, int const nY, int const nWidth, int const nHeight, char const* const lpszDisplayName, char const* const lpszName, int const nActionType, char const* const lpszActionData, char const* const lpszActionHandler)
{
    BUTTON_DATA* lpBd = NULL;
    size_t const uNameLength          = strlen(lpszName);
    size_t const uActionDataLength    = strlen(lpszActionData);
    size_t const uActionHandlerLength = strlen(lpszActionHandler);

    if(MemT2Alloc(&lpBd, lpszName, uNameLength+1U+uActionDataLength+1U+uActionHandlerLength+1U))
    {
        HWND hWnd = CreateWindowExA(0, WC_BUTTONA, lpszDisplayName, WS_CHILD|WS_VISIBLE|(l_hDefaultBtnWnd==NULL ? BS_DEFPUSHBUTTON : BS_PUSHBUTTON), nX, nY, nWidth, nHeight, hWndParent, (HMENU)AddAtomA(lpszName), GetWindowInstance(hWndParent), NULL);

        if(hWnd)
        {
            lpBd->lpszName          = (char*)&lpBd[1];
            lpBd->lpszActionData    = &lpBd->lpszName[uNameLength+1U];
            lpBd->lpszActionHandler = &lpBd->lpszActionData[uActionDataLength+1U];
            lpBd->nActionType       = nActionType;

            strcpy(lpBd->lpszName, lpszName);
            strcpy(lpBd->lpszActionData, lpszActionData);
            strcpy(lpBd->lpszActionHandler, lpszActionHandler);

            SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)lpBd);
            lpBd->lpfnPrevWndProc = SubclassWindow(hWnd, &Button_P_SubclassWndProc);

            if(l_hDefaultBtnWnd==NULL)
            {
                l_hDefaultBtnWnd = hWnd;
            }

            return true;
        }

        MemTFree(&lpBd);
    }

    return false;
}

bool __WDECL ButtonAction(HWND const hWnd, UINT const uBtnId)
{
    HWND hWndButton = GetDlgItem(hWnd, uBtnId);

    while(hWndButton)
    {
        BUTTON_DATA* lpBd = (BUTTON_DATA*)GetWindowLongPtr(hWndButton, GWLP_USERDATA);
        SHELLEXECUTEINFOA Sei = { sizeof(Sei) };

        if(!lpBd)
        {
            break;
        }

        if(lpBd->nActionType==BUTTON_ACTION_SHELLEXEC || lpBd->nActionType==BUTTON_ACTION_SHELLEXEC_CLOSE)
        {
            bool bSuccess = false;
            char* lpszBuf;
            char const* lpszFile;
            char const* lpszParam = "";
            char szFileClass[MAX_REGISTRY_KEY_SIZE+1];

            lpszBuf = MemCrtStrDupA(lpBd->lpszActionData);

            lpszFile = BvStrSkipQuoteA(lpszBuf, 0, &lpszParam);

            if(lpszFile)
            {
                Sei.fMask        = SEE_MASK_FLAG_NO_UI|(ISUNCPATH(lpszFile) ? SEE_MASK_CONNECTNETDRV : 0);
                Sei.hwnd         = hWnd;
                Sei.lpVerb       = NULL;
                Sei.lpFile       = lpszFile;
                Sei.lpParameters = lpszParam;
                Sei.nShow        = SW_SHOWNORMAL;

                if(lpBd->lpszActionHandler[0])
                {
                    if(GetFileClassFromExtension(lpBd->lpszActionHandler, szFileClass, __ARRAYSIZE(szFileClass)))
                    {
                        Sei.lpClass = szFileClass;
                    }
                    else
                    {// not a recognized extension, maybe not an extension at all
                        Sei.lpClass = lpBd->lpszActionHandler;
                    }

                    // override file class
                    Sei.fMask|= SEE_MASK_CLASSNAME;
                }

                bSuccess = ShellExecuteExA(&Sei)!=FALSE;
            }

            MemTFree(&lpszBuf);

            if(!bSuccess)
            {
                break;
            }
        }

        if(lpBd->nActionType==BUTTON_ACTION_SHELLEXEC_CLIENT)
        {
            bool bSuccess = false;
            char* lpszBuf;
            char const* lpszFile;
            char const* lpszParam = "";

            lpszBuf = MemCrtStrDupA(lpBd->lpszActionData);

            lpszFile = BvStrSkipQuoteA(lpszBuf, 0, &lpszParam);

            if(lpszFile)
            {
                bSuccess = StartClient(hWnd, lpszFile, lpszParam);
            }

            MemTFree(&lpszBuf);

            if(!bSuccess)
            {
                break;
            }
        }

        if(lpBd->nActionType==BUTTON_ACTION_MSGBOX)
        {
            char* lpszMsg;
            size_t uLen = strlen(lpBd->lpszActionData)+1U;

            lpszMsg = MemAlloc(uLen);

            if(XF_SlashesSub(lpszMsg, &uLen, lpBd->lpszActionData, NULL))
            {
                MsgBox(hWnd, lpszMsg, MB_OK);
            }

            MemTFree(&lpszMsg);
        }

        if(lpBd->nActionType==BUTTON_ACTION_SHELLEXEC_CLOSE || lpBd->nActionType==BUTTON_ACTION_CLOSE)
        {
            DestroyWindow(hWnd);
        }

        if(lpBd->nActionType==BUTTON_ACTION_MINIMIZE)
        {
            ShowWindow(hWnd, SW_MINIMIZE);
        }

        return true;
    }

    return false;
}

bool __WDECL ButtonCheckName(char const* const lpszName)
{
    char const* lpszIdx;

    // validate button name /^[A-Z0-9_]+$/
    for(lpszIdx = lpszName; (lpszIdx[0]>='A' && lpszIdx[0]<='Z') || (lpszIdx[0]>='0' && lpszIdx[0]<='9') || lpszIdx[0]=='_'; lpszIdx++);

    return (bool)(0==lpszIdx[0]);
}

char const* __WDECL ButtonGetName(UINT const uBtnId, char* const lpszBuffer, size_t const uBufferSize)
{
    lpszBuffer[0] = 0;

    if(GetAtomNameA(uBtnId, lpszBuffer, uBufferSize)!=0U)
    {
        return lpszBuffer;
    }

    return NULL;
}

UINT __WDECL ButtonGetId(char const* const lpszName)
{
    return FindAtomA(lpszName);
}

HWND __WDECL ButtonGetDefault(void)
{
    return l_hDefaultBtnWnd;
}
