#include <stdio.h>
#include <windows.h>

#include "kbdctype.h"

int __cdecl main(int nArgc, char** lppszArgv)
{
    DWORD dwIdx, dwRead;
    KBDCINPUTDATA Kid[100];
    HANDLE hDevice;

    printf("Device open...");

    hDevice = CreateFileA("\\\\.\\KbdCatch", GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);

    if(hDevice!=INVALID_HANDLE_VALUE)
    {
        printf(" success\n");

        printf("Reading...");

        while(ReadFile(hDevice, Kid, sizeof(Kid), &dwRead, NULL))
        {
            printf(" success, %u bytes (%s):\n", dwRead, (dwRead%sizeof(Kid[0])) ? "ERR" : "OK");

            for(dwIdx = 0; dwIdx<dwRead/sizeof(Kid[0]); dwIdx++)
            {
                printf("- Type: %u, MakeCode: %u, Flags: %u, Reserved: %u, ExtraInfo: %u\n",
                    Kid[dwIdx].DeviceType,
                    Kid[dwIdx].MakeCode,
                    Kid[dwIdx].Flags,
                    Kid[dwIdx].Reserved);
            }

            printf("Reading...");
        }

        printf(" failure %u\n", GetLastError());

        CloseHandle(hDevice);
    }
    else
    {
        printf(" failure %u\n", GetLastError());
    }

    return 0;
}
