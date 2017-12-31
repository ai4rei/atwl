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

#include "button.h"
#include "config.h"
#include "rocred.h"

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

bool __stdcall ButtonCreate(HWND hWndParent, const int nX, const int nY, const int nWidth, const int nHeight, const char* const lpszDisplayName, const char* const lpszName, const int nActionType, const char* const lpszActionData, const char* const lpszActionHandler)
{
    ubyte_t* lpucDataBuffer = NULL;
    BUTTON_DATA* lpBd = NULL;
    const size_t uNameLength          = strlen(lpszName);
    const size_t uActionDataLength    = strlen(lpszActionData);
    const size_t uActionHandlerLength = strlen(lpszActionHandler);
    const size_t uDataBufferSize      = sizeof(lpBd[0])
                                      + __STRINGSIZEEX(lpszName,uNameLength+1U)
                                      + __STRINGSIZEEX(lpszActionData,uActionDataLength+1U)
                                      + __STRINGSIZEEX(lpszActionHandler,uActionHandlerLength+1U)
                                      ;

    if(MemTAllocEx(&lpucDataBuffer, uDataBufferSize))
    {
        HWND hWnd = CreateWindowEx(0, WC_BUTTON, lpszDisplayName, WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON, nX, nY, nWidth, nHeight, hWndParent, (HMENU)AddAtom(lpszName), GetWindowInstance(hWndParent), NULL);

        if(hWnd)
        {
            lpBd = (BUTTON_DATA*)lpucDataBuffer;

            lpBd->lpszName          = (char*)&lpBd[1];
            lpBd->lpszActionData    = &lpBd->lpszName[uNameLength+1U];
            lpBd->lpszActionHandler = &lpBd->lpszActionData[uActionDataLength+1U];
            lpBd->nActionType       = nActionType;

            strcpy(lpBd->lpszName, lpszName);
            strcpy(lpBd->lpszActionData, lpszActionData);
            strcpy(lpBd->lpszActionHandler, lpszActionHandler);

            SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)lpBd);
            lpBd->lpfnPrevWndProc = SubclassWindow(hWnd, &Button_P_SubclassWndProc);

            return true;
        }

        MemTFree(&lpucDataBuffer);
    }

    return false;
}

bool __stdcall ButtonAction(HWND hWnd, const unsigned int uBtnId)
{
    HWND hWndButton = GetDlgItem(hWnd, uBtnId);

    while(hWndButton)
    {
        BUTTON_DATA* lpBd = (BUTTON_DATA*)GetWindowLongPtr(hWndButton, GWLP_USERDATA);
        SHELLEXECUTEINFO Sei = { sizeof(Sei) };

        if(!lpBd)
        {
            break;
        }

        if(lpBd->nActionType==BUTTON_ACTION_SHELLEXEC || lpBd->nActionType==BUTTON_ACTION_SHELLEXEC_CLOSE)
        {
            bool bSuccess = false;
            char* lpszBuf;
            const char* lpszFile;
            const char* lpszParam = "";
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

                bSuccess = ShellExecuteEx(&Sei)!=FALSE;
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
            const char* lpszFile;
            const char* lpszParam = "";

            lpszBuf = MemCrtStrDupA(lpBd->lpszActionData);

            lpszFile = BvStrSkipQuoteA(lpszBuf, 0, &lpszParam);

            if(lpszFile)
            {
                bSuccess = (StartClient(hWnd, lpszFile, lpszParam), true);  // TODO: Real result.
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

bool __stdcall ButtonCheckName(const char* const lpszName)
{
    const char* lpszIdx;

    // validate button name /^[A-Z0-9_]+$/
    for(lpszIdx = lpszName; (lpszIdx[0]>='A' && lpszIdx[0]<='Z') || (lpszIdx[0]>='0' && lpszIdx[0]<='9') || lpszIdx[0]=='_'; lpszIdx++);

    return (bool)(0==lpszIdx[0]);
}

const char* __stdcall ButtonGetName(const unsigned int uBtnId, char* const lpszBuffer, const size_t uBufferSize)
{
    lpszBuffer[0] = 0;

    if(GetAtomNameA(uBtnId, lpszBuffer, uBufferSize)!=0U)
    {
        return lpszBuffer;
    }

    return NULL;
}

unsigned int __stdcall ButtonGetId(const char* const lpszName)
{
    return FindAtomA(lpszName);
}
