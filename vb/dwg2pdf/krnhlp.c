// -----------------------------------------------------------------
// AutoCAD DWG2PDF Automation - Helper Routines
// (c) 2012-2014 Ai4rei/AN
//
// This work is licensed under a
// Creative Commons BY-SA 3.0 Unported License
// http://creativecommons.org/licenses/by-sa/3.0/
//
// -----------------------------------------------------------------

#include <windows.h>

LONG WINAPI KrnHlp_FindFirstChange(LPCSTR lpDirectory)
{
    return (LONG)FindFirstChangeNotification(lpDirectory, 0, FILE_NOTIFY_CHANGE_FILE_NAME);
}

LONG WINAPI KrnHlp_FindNextChange(LONG hChange)
{
    return (LONG)(WaitForSingleObject((HANDLE)hChange, INFINITE)==0 && FindNextChangeNotification((HANDLE)hChange));
}

LONG WINAPI KrnHlp_FindCloseChange(LONG hChange)
{
    return (LONG)FindCloseChangeNotification((HANDLE)hChange);
}

LONG WINAPI KrnHlp_TestExclusiveWrite(LPCSTR lpszFile)
{
    for(;;)
    {
        HANDLE hFile = CreateFile(lpszFile, GENERIC_READ|GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);

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
