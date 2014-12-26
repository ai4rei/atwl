#include <windows.h>

#include <btypes.h>
#include <bvcstr.h>
#include <memory.h>
#include <xf_slash.h>

#include "button.h"
#include "config.h"
#include "rocred.h"

enum BUTTON_ACTION
{
    BUTTON_ACTION_SHELLEXEC = 0,
    BUTTON_ACTION_SHELLEXEC_CLOSE,
    BUTTON_ACTION_CLOSE,
    BUTTON_ACTION_MSGBOX,
    BUTTON_ACTION_MAX
};

static char* l_lpszBtnId2Name = NULL;  // array of indexed zero-terminated character strings (item1\0item2\0\0)

bool __stdcall ButtonAction(HWND hWnd, unsigned int uBtnId)
{
    const char* lpszBtnName = ButtonGetName(uBtnId);

    while(lpszBtnName)
    {
        char szBuffer[4096];
        const char* lpszActionData;
        const char* lpszActionHandler;
        int nActionType;
        SHELLEXECUTEINFO Sei = { sizeof(Sei) };

        wsprintf(szBuffer, "ROCred.Buttons.%s", lpszBtnName);

        nActionType       = ConfigGetIntFromSection(szBuffer, "ActionType");
        lpszActionData    = ConfigGetStrFromSection(szBuffer, "ActionData");
        lpszActionHandler = ConfigGetStrFromSection(szBuffer, "ActionHandler");

        if(lpszActionData[0]=='#')
        {
            LoadStringA(GetModuleHandle(NULL), BvStrToULongA(lpszActionData+1, NULL, 10), szBuffer, __ARRAYSIZE(szBuffer));
            lpszActionData = szBuffer;
        }

        if(nActionType==BUTTON_ACTION_SHELLEXEC || nActionType==BUTTON_ACTION_SHELLEXEC_CLOSE)
        {
            char* lpszBuf;
            char* lpszIdx;
            const char* lpszFile;
            const char* lpszParam = "";
            char szFileClass[MAX_REGISTRY_KEY_SIZE+1];
            BOOL bSuccess;

            lpszIdx = lpszBuf = Memory_DuplicateString(lpszActionData);

            if(lpszIdx[0]=='"')
            {
                for(lpszFile = ++lpszIdx; lpszIdx[0] && lpszIdx[0]!='"'; lpszIdx++);

                if(lpszIdx[0]=='"' && lpszIdx[1]==' ')
                {
                    lpszParam = lpszIdx+2;
                }

                lpszIdx[0] = 0;
            }
            else
            {
                for(lpszFile = lpszIdx; lpszIdx[0] && lpszIdx[0]!=' '; lpszIdx++);

                if(lpszIdx[0]==' ')
                {
                    lpszParam = lpszIdx+1;
                }

                lpszIdx[0] = 0;
            }

            Sei.fMask        = SEE_MASK_FLAG_NO_UI|(ISUNCPATH(lpszFile) ? SEE_MASK_CONNECTNETDRV : 0);
            Sei.hwnd         = hWnd;
            Sei.lpVerb       = "open";
            Sei.lpFile       = lpszFile;
            Sei.lpParameters = lpszParam;
            Sei.nShow        = SW_SHOWNORMAL;

            if(lpszActionHandler[0])
            {
                if(GetFileClassFromExtension(lpszActionHandler, szFileClass, __ARRAYSIZE(szFileClass)))
                {
                    Sei.lpClass = szFileClass;
                }
                else
                {// not a recognized extension, maybe not an extension at all
                    Sei.lpClass = lpszActionHandler;
                }

                // override file class
                Sei.fMask|= SEE_MASK_CLASSNAME;
            }

            bSuccess = ShellExecuteEx(&Sei);

            Memory_FreeEx(&lpszBuf);

            if(!bSuccess)
            {
                break;
            }
        }

        if(nActionType==BUTTON_ACTION_MSGBOX)
        {
            char* lpszMsg;
            size_t uLen = lstrlenA(lpszActionData)+1;

            lpszMsg = Memory_Alloc(uLen);

            if(XF_SlashesSub(lpszMsg, &uLen, lpszActionData, NULL))
            {
                MsgBox(hWnd, lpszMsg, MB_OK);
            }

            Memory_FreeEx(&lpszMsg);
        }

        if(nActionType==BUTTON_ACTION_SHELLEXEC_CLOSE || nActionType==BUTTON_ACTION_CLOSE)
        {
            DestroyWindow(hWnd);
        }

        return true;
    }

    return false;
}

bool __stdcall ButtonCheckName(const char* lpszName)
{
    const char* lpszIdx;

    // validate button name /^[A-Z0-9_]+$/
    for(lpszIdx = lpszName; (lpszIdx[0]>='A' && lpszIdx[0]<='Z') || (lpszIdx[0]>='0' && lpszIdx[0]<='9') || lpszIdx[0]=='_'; lpszIdx++);

    return (bool)(0==lpszIdx[0]);
}

const char* __stdcall ButtonGetName(unsigned int uBtnId)
{
    const char* lpszIdx;
    unsigned int uId = 0;

    if(l_lpszBtnId2Name)
    {
        for(lpszIdx = l_lpszBtnId2Name; lpszIdx[0] && uId!=uBtnId; lpszIdx = &lpszIdx[lstrlenA(lpszIdx)+1], uId++);

        if(lpszIdx[0])
        {// found
            return lpszIdx;
        }
    }

    return NULL;
}

unsigned int __stdcall ButtonGetId(const char* lpszName)
{
    // NOTE: This is not very efficient method to do it, but we are
    //       not doing anything that asks for high performance...
    const char* lpszIdx;
    unsigned int uId = 0;
    size_t uLen = 0, uNameLen;

    if(l_lpszBtnId2Name)
    {
        for(lpszIdx = l_lpszBtnId2Name; lpszIdx[0] && lstrcmp(lpszIdx, lpszName); lpszIdx = &lpszIdx[lstrlenA(lpszIdx)+1], uId++);

        if(lpszIdx[0])
        {// found
            return uId;
        }

        uLen = lpszIdx-l_lpszBtnId2Name;
    }

    uNameLen         = lstrlenA(lpszName);
    l_lpszBtnId2Name = Memory_Realloc(l_lpszBtnId2Name, sizeof(l_lpszBtnId2Name)*(uLen+1+uNameLen+1));

    lstrcpyA(&l_lpszBtnId2Name[uLen], lpszName);
    l_lpszBtnId2Name[uLen+1+uNameLen] = 0; // EOA

    return uId;
}

void __stdcall ButtonFree(void)
{
    if(l_lpszBtnId2Name)
    {
        Memory_FreeEx(&l_lpszBtnId2Name);
    }
}
