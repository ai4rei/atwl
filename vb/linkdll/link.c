/* -----------------------------------------------------------------
// MIM LINK.EXE to allow DLL building with VB6
//
// Usage:
//  Rename original to KNIL.EXE and place the compiled LINK.EXE in
//  it's stead.
//
// License:
//  Public Domain.
//
// Original concepts:
//  http://www.windowsdevcenter.com/pub/a/windows/2005/04/26/create_dll.html
//  http://www.hermetic.ch/vbm2dll.htm
//
// ---------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#define DLL_TEXT " /DEF:VBADLL.DEF"

int main(int nArgc, char** lppszArgv)
{
    int nIdx, nResult = EXIT_FAILURE;
    char* lpszNewCmd;
    const char* lpszCmdLine;

    for(nIdx = 1; nIdx<nArgc; nIdx++)
    {
        if(!strcmp(lppszArgv[nIdx], "/DLL"))
        {
            break;
        }
    }

    lpszCmdLine = GetCommandLineA();

    if(nIdx==nArgc)
    {
        lpszNewCmd = strdup(lpszCmdLine);
    }
    else
    {
        lpszNewCmd = malloc(strlen(lpszCmdLine)+sizeof(DLL_TEXT));

        if(lpszNewCmd)
        {
            sprintf(lpszNewCmd, "%s%s", lpszCmdLine, DLL_TEXT);
        }
    }

    if(lpszNewCmd)
    {
        char* lpszName = strstr(lpszNewCmd, "LINK");

        if(lpszName)
        {
            STARTUPINFO Si = { sizeof(Si) };
            PROCESS_INFORMATION Pi;

            ((long*)lpszName)[0] = 'LINK';  /* reverse name of actual linker */

            if(CreateProcess(NULL, lpszNewCmd, NULL, NULL, FALSE, CREATE_SUSPENDED, NULL, NULL, &Si, &Pi))
            {
                ResumeThread(Pi.hThread);
                WaitForSingleObject(Pi.hProcess, INFINITE);
                GetExitCodeProcess(Pi.hProcess, &nResult);
                CloseHandle(Pi.hThread);
                CloseHandle(Pi.hProcess);
            }
        }

        free(lpszNewCmd);
    }

    return nResult;
}
