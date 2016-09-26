#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>
#include <tchar.h>
#include <wchar.h>

INT _tmain(INT nArgc, LPTSTR* lppszArgv)
{
    BOOL bShowHelp = FALSE, bFileOnly = FALSE;
    INT nIdx, nResult = EXIT_FAILURE;
    LPCTSTR lpszMask = NULL;
    SHFILEOPSTRUCT Fo = { 0 };
    LPTSTR lpszzMask;
    UINT uMaskLen;

    for(nIdx = 1; nIdx<nArgc; nIdx++)
    {
        LPCTSTR lpszArg = lppszArgv[nIdx];

        if(lpszArg[0]=='/' || lpszArg[0]=='-')
        {// switches
            lpszArg++;

            if(_tcscmp(lpszArg, _T("?"))==0 || _tcscmp(lpszArg, _T("h"))==0 || _tcscmp(lpszArg, _T("help"))==0)
            {
                bShowHelp = TRUE;
                break;
            }
            else
            if(_tcscmp(lpszArg, _T("f"))==0)
            {
                bFileOnly = TRUE;
            }
            else
            {
                _tprintf(_T("Unknown switch `%s'.\n"), lpszArg);

                return EXIT_FAILURE;
            }
        }
        else
        {// parameters
            if(!lpszMask)
            {
                lpszMask = lpszArg;
            }
            else
            {
                _tprintf(_T("Too many parameters.\n"));

                return EXIT_FAILURE;
            }
        }
    }

    if(!lpszMask)
    {
        _tprintf(_T("Insufficient parameters.\n\n"));
    }

    if(bShowHelp || !lpszMask)
    {
        _tprintf(
            _T("Usage: recycle [-?] [-f] <path>\n\n")
            _T("\n")
            _T("Switches:\n")
            _T("\t-? or -h or -help\n")
            _T("\tShows usage information.\n")
            _T("\n")
            _T("\t-f\n")
            _T("\tDeletes only files.\n")
            _T("\n")
            _T("Parameters:\n")
            _T("\t<path>\n")
            _T("\tFile/directory to move to recycle bin. Can include wildcards.\n")
        );

        return bShowHelp ? EXIT_SUCCESS : EXIT_FAILURE;
    }

    uMaskLen = _tcslen(lpszMask);

    if(!uMaskLen)
    {/* this will happen if you pass "" as parameter */
        _tprintf(_T("Insufficient parameters.\n\n"));

        return EXIT_FAILURE;
    }

    lpszzMask = (LPTSTR)LocalAlloc(LMEM_FIXED, (uMaskLen+2U)*sizeof(lpszzMask[0]));

    if(lpszzMask)
    {
        _tcscpy(lpszzMask, lpszMask);
        lpszzMask[uMaskLen+1U] = 0;  /* double-zero */

        Fo.wFunc = FO_DELETE;
        Fo.pFrom = lpszzMask;
        Fo.fFlags = FOF_ALLOWUNDO|FOF_NOCONFIRMATION|( bFileOnly ? FOF_FILESONLY : 0);

        if(SHFileOperation(&Fo)!=ERROR_SUCCESS)
        {
            _tprintf(_T("File operation failed.\n"));
        }
        else
        if(Fo.fAnyOperationsAborted)
        {
            _tprintf(_T("File operation aborted.\n"));
        }
        else
        {
            nResult = EXIT_SUCCESS;
        }

        LocalFree(lpszzMask);
        lpszzMask = NULL;
    }
    else
    {
        _tprintf(_T("Insufficient memory.\n"));
    }

    return nResult;
}
