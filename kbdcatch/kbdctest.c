#include <stdio.h>
#include <windows.h>

int __cdecl main(int nArgc, char** lppszArgv)
{
    HANDLE hDevice;

    printf("Device open...");

    hDevice = CreateFileA("\\\\.\\KbdCatch", GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);

    if(hDevice!=INVALID_HANDLE_VALUE)
    {
        printf("success\n");

        Sleep(1000);

        CloseHandle(hDevice);
    }
    else
    {
        printf("failure\n");
    }

    return 0;
}
