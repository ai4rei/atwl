/* -----------------------------------------------------------------
// AutoCAD DWG2PDF Automation - Support Library
// (c) 2012-2014 Ai4rei/AN
//
// This work is licensed under a
// Creative Commons BY-SA 3.0 Unported License
// http://creativecommons.org/licenses/by-sa/3.0/
//
// ---------------------------------------------------------------*/

#include <windows.h>

BOOL WINAPI DWG2PDF_P_TestExclusiveWrite(LPCSTR lpszFile)
{
    for(;;)
    {
        HANDLE hFile = CreateFile(lpszFile, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);

        if(hFile==INVALID_HANDLE_VALUE)
        {
            if(GetLastError()==ERROR_FILE_NOT_FOUND)
            {
                return FALSE;
            }
        }
        else
        {
            CloseHandle(hFile);
            return TRUE;
        }

        Sleep(1000);
    }
}

BOOL WINAPI DWG2PDF_P_EnumFiles(LPCSTR lpszDirectory, LONG (WINAPI* Func)(BSTR lpszFilePath, LONG nContext), LONG nContext)
{
    BOOL bSuccess = TRUE;
    CHAR szPathMask[MAX_PATH];
    HANDLE hFind;
    WIN32_FIND_DATA Wfd;

    wsprintf(szPathMask, "%s\\*", lpszDirectory);

    if((hFind = FindFirstFile(szPathMask, &Wfd))!=INVALID_HANDLE_VALUE)
    {
        do
        {
            if(Wfd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
            {/* directory */
                if(Wfd.cFileName[0]=='.' && ((Wfd.cFileName[1]=='.' && !Wfd.cFileName[2]) || !Wfd.cFileName[1]))
                {/* . & .. */
                    continue;
                }

                wsprintf(szPathMask, "%s\\%s", lpszDirectory, Wfd.cFileName);

                if(!DWG2PDF_P_EnumFiles(szPathMask, Func, nContext))
                {
                    bSuccess = FALSE;
                    break;
                }
            }
            else
            {/* file */
                wsprintf(szPathMask, "%s\\%s", lpszDirectory, Wfd.cFileName);

                if(DWG2PDF_P_TestExclusiveWrite(szPathMask))
                {
                    BSTR bsFilePath;
                    WCHAR wszFilePath[MAX_PATH*4];

                    MultiByteToWideChar(CP_ACP, 0, szPathMask, -1, wszFilePath, sizeof(wszFilePath)/sizeof(wszFilePath[0]));
                    bsFilePath = SysAllocString(wszFilePath);

                    bSuccess = Func(bsFilePath, nContext);

                    SysFreeString(bsFilePath);

                    if(!bSuccess)
                    {
                        break;
                    }
                }
            }
        }
        while(FindNextFile(hFind, &Wfd));

        FindClose(hFind);
    }

    return bSuccess;
}

VOID WINAPI DWG2PDF_ProcessFiles(LPCSTR lpszDirectory, LONG (WINAPI* Func)(BSTR lpszFilePath, LONG nContext), LONG nContext)
{
    HANDLE hChange = FindFirstChangeNotification(lpszDirectory, TRUE, FILE_NOTIFY_CHANGE_FILE_NAME);

    if(hChange!=INVALID_HANDLE_VALUE)
    {
        do
        {
            Sleep(1000);

            if(!DWG2PDF_P_EnumFiles(lpszDirectory, Func, nContext))
            {
                break;
            }
        }
        while(WaitForSingleObject(hChange, INFINITE)==WAIT_OBJECT_0 && FindNextChangeNotification(hChange));

        FindCloseChangeNotification(hChange);
    }
}
