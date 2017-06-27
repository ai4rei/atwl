#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>

#include <windows.h>

int __cdecl main(int argc, char** argv)
{
    WIN32_FIND_DATAW Wfd;
    HANDLE hFind = FindFirstFileW(L"*", &Wfd);

    if(hFind!=INVALID_HANDLE_VALUE)
    {
        do
        {
            wchar_t szBuffer[MAX_PATH*2+36];
            WIN32_FIND_STREAM_DATA Wfsd;
            HANDLE hFads;

            if(Wfd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
            {
                continue;
            }

#ifdef HAVE_FINDFIRSTSTREAM
            hFads = FindFirstStreamW(Wfd.cFileName, FindStreamInfoStandard, &Wfsd, 0);

            if(hFads!=INVALID_HANDLE_VALUE)
            {
                do
                {
                    swprintf(szBuffer, _ARRAYSIZE(szBuffer), L"%s%s", Wfd.cFileName, Wfsd.cStreamName);

                    wprintf(L"%s ", szBuffer);

                    if(wcscmp(Wfsd.cStreamName, L":Zone.Identifier:$DATA")==0)
                    {
                        if(DeleteFileW(szBuffer))
                        {
                            wprintf(L"deleted");
                        }
                        else
                        {
                            wprintf(L"failed");
                        }
                    }
                    else
                    {
                        wprintf(L"skipped");
                    }

                    wprintf(L"\n");
                }
                while(FindNextStreamW(hFads, &Wfsd));

                FindClose(hFads);
            }
#else  /* HAVE_FINDFIRSTSTREAM */
            swprintf(szBuffer, _ARRAYSIZE(szBuffer), L"%s:Zone.Identifier:$DATA", Wfd.cFileName);

            if(DeleteFileW(szBuffer))  /* if you cannot enumerate, then shoot blindly */
            {
                wprintf(L"%s deleted", szBuffer);
            }
            else
            if(GetLastError()!=ERROR_FILE_NOT_FOUND)
            {
                wprintf(L"%s failed", szBuffer);
            }
#endif  /* HAVE_FINDFIRSTSTREAM */
        }
        while(FindNextFileW(hFind, &Wfd));

        FindClose(hFind);
    }

    return EXIT_SUCCESS;
}
