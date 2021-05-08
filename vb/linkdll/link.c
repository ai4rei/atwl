/* -----------------------------------------------------------------
// MIN LINK.EXE to allow DLL building with VB6
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
#include <tchar.h>

#define DLL_TEXT _T(" /DEF:VBADLL.DEF")

INT __cdecl _tmain(INT nArgc, TCHAR** lppszArgv)
{
    int nIdx, nResult = EXIT_FAILURE;
    TCHAR* lpszNewCmd;
    const TCHAR* lpszCmdLine;

    for(nIdx = 1; nIdx<nArgc; nIdx++)
    {
        if(_tcscmp(lppszArgv[nIdx], _T("/DLL"))==0)
        {
            break;
        }
    }

    lpszCmdLine = GetCommandLine();

    if(nIdx==nArgc)
    {
        lpszNewCmd = _tcsdup(lpszCmdLine);
    }
    else
    {
        lpszNewCmd = malloc(sizeof(lpszNewCmd[0])*(_tcslen(lpszCmdLine)+_tcslen(DLL_TEXT)+1U));

        if(lpszNewCmd!=NULL)
        {
            _stprintf(lpszNewCmd, _T("%s%s"), lpszCmdLine, DLL_TEXT);
        }
    }

    if(lpszNewCmd!=NULL)
    {
        TCHAR* lpszName = _tcsstr(lpszNewCmd, _T("LINK"));

        if(lpszName!=NULL)
        {
            STARTUPINFO Si = { sizeof(Si) };
            PROCESS_INFORMATION Pi;

            /* reverse name of actual linker */
            lpszName[0] = _T('K');
            lpszName[1] = _T('N');
            lpszName[2] = _T('I');
            lpszName[3] = _T('L');

            if(CreateProcess(NULL, lpszNewCmd, NULL, NULL, FALSE, 0, NULL, NULL, &Si, &Pi))
            {
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
