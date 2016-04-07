#include <stdio.h>
#include <windows.h>
#include <tchar.h>

INT __cdecl _tmain(INT nArgc, LPTSTR* lppszArgv)
{
    bool bCreate = false;
    bool bAccess = false;
    bool bChange = false;
    bool bExists = false;
    LPCTSTR lpszFileName = NULL;
    FILETIME FT;
    HANDLE hFile;

    for(int nIdx = 1; nIdx<nArgc; nIdx++)
    {
        LPCTSTR lpszArg = lppszArgv[nIdx];

        if(lpszArg[0]=='-')
        {
            lpszArg++;

            if(_tcscmp(lpszArg, _T("c"))==0)
            {
                bCreate = true;
            }
            else
            if(_tcscmp(lpszArg, _T("m"))==0)
            {
                bChange = true;
            }
            else
            if(_tcscmp(lpszArg, _T("a"))==0)
            {
                bAccess = true;
            }
            else
            if(_tcscmp(lpszArg, _T("e"))==0)
            {
                bExists = true;
            }
            else
            {
                _tprintf(_T("Unknown switch `%s'.\n"), lpszArg);
                return EXIT_FAILURE;
            }
        }
        else
        if(!lpszFileName)
        {
            lpszFileName = lpszArg;
        }
        else
        {
            _tprintf(_T("Too many parameters.\n"));
            return EXIT_FAILURE;
        }
    }

    if(!lpszFileName || !(bCreate || bChange || bAccess))
    {
        _tprintf(_T("Usage: touch <filename> [-c] [-m] [-a] [-e]\n"));
        return EXIT_FAILURE;
    }

    INT nResult = EXIT_FAILURE;

    GetSystemTimeAsFileTime(&FT);

    if((hFile = CreateFile(lpszFileName, GENERIC_WRITE, FILE_SHARE_READ, NULL, bExists ? OPEN_EXISTING : OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL))!=INVALID_HANDLE_VALUE)
    {
        if(SetFileTime(hFile, bCreate ? &FT : NULL, bAccess ? &FT : NULL, bChange ? &FT : NULL))
        {
            nResult = EXIT_SUCCESS;
        }
        else
        {
            _tprintf(_T("Failed to set time information (code=%u).\n"), GetLastError());
        }

        CloseHandle(hFile);
    }
    else
    {
        _tprintf(_T("Failed to open file for writing (code=%u).\n"), GetLastError());
    }

    return nResult;
}
