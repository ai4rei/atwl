#include <stdio.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <tchar.h>

void UpdateSSV(LPCTSTR lpszFileName)
{
    HANDLE hFile = CreateFile(lpszFileName, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_RANDOM_ACCESS, NULL);

    if(hFile==INVALID_HANDLE_VALUE)
    {
        _tprintf(_T("Error: Cannot open file `%s'.\n"), lpszFileName);
    }
    else
    {
        HANDLE hMap = CreateFileMapping(hFile, NULL, PAGE_READWRITE, 0, 0, NULL);

        if(hMap==NULL)
        {
            _tprintf(_T("Error: Cannot map file `%s'.\n"), lpszFileName);
        }
        else
        {
            PIMAGE_DOS_HEADER lpIDH = NULL;
            LPVOID lpINHV = NULL;

            _tprintf(_T("%s\n"), lpszFileName);

            for(;;)
            {
                PIMAGE_NT_HEADERS32 lpINH;

                lpIDH = MapViewOfFile(hMap, FILE_MAP_READ, 0, 0, sizeof(lpIDH[0]));

                if(lpIDH==NULL)
                {
                    _tprintf(_T("Error: Cannot map view of DOS header.\n"));
                    break;
                }

                if(lpIDH->e_magic!=IMAGE_DOS_SIGNATURE)
                {
                    _tprintf(_T("Error: DOS header signature not found.\n"));
                    break;
                }

                lpINHV = MapViewOfFile(hMap, FILE_MAP_WRITE, 0, 0, lpIDH->e_lfanew+sizeof(lpINH[0]));

                if(lpINHV==NULL)
                {
                    _tprintf(_T("Error: Cannot map view of NT header.\n"));
                    break;
                }

                lpINH = (LPVOID)(((LPBYTE)lpINHV)+lpIDH->e_lfanew);

                if(lpINH->Signature!=IMAGE_NT_SIGNATURE)
                {
                    _tprintf(_T("Error: NT header signature not found.\n"));
                    break;
                }

                if(lpINH->OptionalHeader.Magic!=IMAGE_NT_OPTIONAL_HDR32_MAGIC)
                {
                    _tprintf(_T("Error: NT optional header signature not found.\n"));
                    break;
                }

                _tprintf(_T("Current sub-system version: %u.%u\n"), lpINH->OptionalHeader.MajorSubsystemVersion, lpINH->OptionalHeader.MinorSubsystemVersion);

                for(;;)
                {
                    TCHAR szBuffer[16];

                    _tprintf(_T("New sub-system version: "));

                    if(_fgetts(szBuffer, _ARRAYSIZE(szBuffer), stdin))
                    {
                        int n;
                        WORD wMajor, wMinor;

                        if(szBuffer[0]=='\r' || szBuffer[0]=='\n' || szBuffer[0]=='\0')
                        {
                            break;
                        }

                        n = _stscanf(szBuffer, _T("%hu.%hu"), &wMajor, &wMinor);

                        if(n==2)
                        {
                            lpINH->OptionalHeader.MajorSubsystemVersion = wMajor;
                            lpINH->OptionalHeader.MinorSubsystemVersion = wMinor;
                            break;
                        }
                    }
                }

                break;
            }

            if(lpINHV)
            {
                UnmapViewOfFile(lpINHV);
            }
            if(lpIDH)
            {
                UnmapViewOfFile(lpIDH);
            }

            CloseHandle(hMap);
        }

        CloseHandle(hFile);
    }
}

INT _tmain(INT nArgc, TCHAR** lppszArgv)
{
    INT nIdx;

    if(nArgc<2)
    {
        _tprintf(_T("Usage: SetSubSystemVersion <PE file> [<PE file> ...]\n"));
        return 0;
    }

    for(nIdx = 1; nIdx<nArgc; nIdx++)
    {
        UpdateSSV(lppszArgv[nIdx]);
    }

    return 0;
}
