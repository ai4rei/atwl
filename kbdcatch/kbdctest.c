#include <stdio.h>
#include <windows.h>

typedef struct KEYBOARD_INPUT_DATA {
    USHORT UnitId;   // zero-based unit number of the keyboard port
    USHORT MakeCode; // the make scan code (key depression)
    USHORT Flags;    // indicates a break (key release) and
                     // other scan-code info
    USHORT Reserved;
    ULONG ExtraInformation; // device-specific additional
                            // information for the event
} KEYBOARD_INPUT_DATA, *PKEYBOARD_INPUT_DATA;

int __cdecl main(int nArgc, char** lppszArgv)
{
    DWORD dwIdx, dwRead;
    KEYBOARD_INPUT_DATA Kid[100];
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
                printf("- UnitId: %u, MakeCode: %u, Flags: %u, Reserved: %u, ExtraInfo: %u\n",
                    Kid[dwIdx].UnitId,
                    Kid[dwIdx].MakeCode,
                    Kid[dwIdx].Flags,
                    Kid[dwIdx].Reserved,
                    Kid[dwIdx].ExtraInformation);
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
