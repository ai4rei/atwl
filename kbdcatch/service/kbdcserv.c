#include <stdio.h>

#include <windows.h>
#include <accctrl.h>
#include <aclapi.h>

#include <btypes.h>
#include <bvargs.h>
#include <bvdebug.h>
#include <bvsque.h>
#include <bvsvcs.h>

#include "../kbdctype.h"

#define KBDCSERV_SIZE 100
#define KBDCSERV_PIPENAME "\\\\.\\pipe\\KbdcData"
#define KBDCSERV_PIPESIZE 10

extern LPCWSTR SERVICE_NAME                 = L"KbdcServ";
extern LPCWSTR SERVICE_INSTALL_DISPLAYNAME  = L"Keyboard Catcher Service";
extern LPCWSTR SERVICE_INSTALL_DEPENDENCIES = L"\0";
extern LPCWSTR SERVICE_INSTALL_USERNAME     = L"NT AUTHORITY\\LocalService";
extern LPCWSTR SERVICE_INSTALL_PASSWORD     = NULL;
extern BOOL    SERVICE_INSTALL_DELAYSTART   = TRUE;

#ifdef _DEBUG
    #define KbdcPrint(_x_) BvDbgPrintf _x_
#else  /* _DEBUG */
    #define KbdcPrint(_x_)
#endif  /* _DEBUG */

#pragma pack(push,1)
typedef struct _SERVICEDATA
{
    SERVICE_STATUS Status;
    SERVICE_STATUS_HANDLE StatusHandle;
}
SERVICEDATA,* PSERVICEDATA;

typedef struct _KBDCSERVKEYSTATE
{
    UBIT_T(ALT,2);
    UBIT_T(CTRL,2);
    UBIT_T(SHIFT,2);
    UBIT_T(CAPSLOCK,1);
    UBIT_T(NUMLOCK,1);
    UBIT_T(SCROLLLOCK,1);
}
KBDCSERVKEYSTATE,* LPKBDCSERVKEYSTATE;

typedef struct _KBDCSERVDEVICESTATE
{
    KBDCSERVKEYSTATE KeyState;
    USHORT DeviceType;
    BYTE CharBuffer[KBDCSERV_SIZE];
    ULONG CharBufferLength;
}
KBDCSERVDEVICESTATE,* LPKBDCSERVDEVICESTATE;

typedef struct _KBDCSERVSTATE
{
    HANDLE hExitEvent;
    HANDLE hStopEvent;
    SERVICEDATA ServiceData;
    BOOLEAN ServiceMode;
    BOOLEAN ServiceCreate;
    BOOLEAN ServiceDelete;
    BOOLEAN SessionLocked;
    KBDCSERVDEVICESTATE DevState[KBDC_DEVTYPE_MAX];
    BVSQUE PipeQueue;
    CRITICAL_SECTION PipeCS;
}
KBDCSERVSTATE,* PKBDCSERVSTATE;
#pragma pack(pop)

CONST BYTE l_ScanCodeToAsciiN[] =
{/* English Keyboard Layout (normal) */
    000, 000, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 000,'\t',  /* 00~15 */
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']','\n', 000, 'a', 's',  /* 16~31 */
    'd', 'f', 'g', 'h', 'j', 'k', 'l', ';','\'', '`', 000,'\\', 'z', 'x', 'c', 'v',  /* 32~47 */
    'b', 'n', 'm', ',', '.', '/', 000, '*', 000, ' ', 000, 000, 000, 000, 000, 000,  /* 48~63 */
    000, 000, 000, 000, 000, 000, 000, '7', '8', '9', '-', '4', '5', '6', '+', '1',  /* 64~79 */
    '2', '3', '0', '.', 000, 000,'\\', 000, 000, 000, 000, 000, 000, 000, 000, 000,  /* 80~95 */
};
CONST BYTE l_ScanCodeToAsciiS[] =
{/* English Keyboard Layout (shift) */
    000, 000, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', 000,'\t',  /* 00~15 */
    'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}','\n', 000, 'A', 'S',  /* 16~31 */
    'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~', 000, '|', 'Z', 'X', 'C', 'V',  /* 32~47 */
    'B', 'N', 'M', '<', '>', '?', 000, '*', 000, ' ', 000, 000, 000, 000, 000, 000,  /* 48~63 */
    000, 000, 000, 000, 000, 000, 000, '7', '8', '9', '-', '4', '5', '6', '+', '1',  /* 64~79 */
    '2', '3', '0', '.', 000, 000, '|', 000, 000, 000, 000, 000, 000, 000, 000, 000,  /* 80~95 */
};

KBDCSERVSTATE l_State = { 0 };

VOID __WDECL KbdcServPipeBroadcastData(PVOID pData, DWORD dwSize)
{
    size_t uIdx;

    EnterCriticalSection(&l_State.PipeCS);

    for(uIdx = 0; uIdx<BVSQueLength(&l_State.PipeQueue); uIdx++)
    {
        DWORD dwRead;
        HANDLE hPipe = BVSQueAt(&l_State.PipeQueue, uIdx)[0];
        OVERLAPPED Ovl = { 0, 0, 0, 0, NULL };

        if(!WriteFile(hPipe, pData, dwSize, &dwRead, &Ovl))
        {
            if(GetLastError()!=ERROR_IO_PENDING)
            {
                KbdcPrint(("WriteFile in KbdcServPipeBroadcastData failed (code=%#x).\n", GetLastError()));
                BVSQueEraseByEntry(&l_State.PipeQueue, hPipe);
                DisconnectNamedPipe(hPipe);
                CloseHandle(hPipe);
                uIdx--;
                continue;
            }
        }

        KbdcPrint(("Pipe %p schuduled to write packet.\n", hPipe));
    }

    LeaveCriticalSection(&l_State.PipeCS);
}

VOID __WDECL KbdcServPipeBroadcastPacket(LPKBDCSERVDEVICESTATE pDS)
{
    if(l_State.SessionLocked)
    {
        KbdcPrint(("KbdcServPipeBroadcastPacket: Session locked, discarding packet.\n"));
        return;
    }

    KbdcServPipeBroadcastData(&pDS->DeviceType, sizeof(pDS->DeviceType)+pDS->CharBufferLength);
}

BOOL __WDECL KbdcServPipeConnect(HANDLE hPipe)
{
    BOOL bSuccess = FALSE;
    HANDLE hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    if(hEvent)
    {
        for(;;)
        {
            OVERLAPPED Ovl = { 0, 0, 0, 0, hEvent };

            if(ConnectNamedPipe(hPipe, &Ovl))
            {
                ;
            }
            else if(GetLastError()==ERROR_PIPE_CONNECTED)
            {
                ;
            }
            else if(GetLastError()==ERROR_IO_PENDING)
            {
                DWORD dwWait, dwDummy;
                HANDLE Events[] =
                {
                    hEvent,
                    l_State.hStopEvent,
                    l_State.hExitEvent,
                };

                dwWait = WaitForMultipleObjects(__ARRAYSIZE(Events), Events, FALSE, INFINITE);

                if(dwWait==WAIT_OBJECT_0+1 || dwWait==WAIT_ABANDONED_0+1)
                {/* hStopEvent */
                    bSuccess = TRUE;
                    break;
                }

                if(dwWait==WAIT_OBJECT_0+2 || dwWait==WAIT_ABANDONED_0+2)
                {/* hExitEvent */
                    bSuccess = TRUE;
                    break;
                }

                if(dwWait==WAIT_FAILED)
                {
                    KbdcPrint(("WaitForMultipleObjects during ConnectNamedPipe completion failed (code=%#x).\n", GetLastError()));
                    break;
                }

                /* WAIT_OBJECT_0 == hEvent */
                if(!GetOverlappedResult(hPipe, &Ovl, &dwDummy, FALSE))
                {
                    KbdcPrint(("GetOverlappedResult during ConnectNamedPipe completion failed (code=%#x).\n", GetLastError()));
                    break;
                }
            }
            else
            {
                break;
            }

            EnterCriticalSection(&l_State.PipeCS);

            /*
                check for broken pipes
            */
            KbdcServPipeBroadcastData("", 0);

            BVSQuePushTail(&l_State.PipeQueue, hPipe);

            LeaveCriticalSection(&l_State.PipeCS);

            bSuccess = TRUE;
            break;
        }

        CloseHandle(hEvent);
    }

    return bSuccess;
}

VOID __WDECL KbdcServPipeDisconnectAll(VOID)
{
    EnterCriticalSection(&l_State.PipeCS);

    while(BVSQueLength(&l_State.PipeQueue))
    {
        HANDLE hPipe = BVSQuePopTail(&l_State.PipeQueue);

        FlushFileBuffers(hPipe);
        DisconnectNamedPipe(hPipe);
        CloseHandle(hPipe);
    }

    LeaveCriticalSection(&l_State.PipeCS);
}

DWORD CALLBACK KbdcServPipeManager(LPVOID lpParam)
{
    KbdcPrint(("Pipe manager thread started.\n"));

    for(;;)
    {
        DWORD dwError;
        EXPLICIT_ACCESS ExAcc = { 0 };
        PACL pACL = NULL;
        /*
            Pipe is created as duplex, even though it is only out-
            bound. This is because the client needs write access for
            SetNamedPipeHandleState to switch into message mode.
        */
        HANDLE hPipe = CreateNamedPipe(KBDCSERV_PIPENAME, /*PIPE_ACCESS_OUTBOUND|*/PIPE_ACCESS_DUPLEX|FILE_FLAG_OVERLAPPED|WRITE_DAC, PIPE_TYPE_MESSAGE|PIPE_READMODE_MESSAGE|PIPE_WAIT, PIPE_UNLIMITED_INSTANCES, KBDCSERV_SIZE, 0, 0, NULL);

        if(hPipe==INVALID_HANDLE_VALUE)
        {
            KbdcPrint(("CreateNamedPipe failed (code=%#x).\n", GetLastError()));
            return EXIT_FAILURE;
        }

        /*
            Enable access for lower integrity clients. Since we want
            to service, when we are a service, don't we?
        */
        ExAcc.grfAccessPermissions = GENERIC_READ|GENERIC_WRITE|SYNCHRONIZE;
        ExAcc.grfAccessMode = SET_ACCESS;
        ExAcc.grfInheritance = NO_INHERITANCE;
        ExAcc.Trustee.TrusteeForm = TRUSTEE_IS_NAME;
        ExAcc.Trustee.TrusteeType = TRUSTEE_IS_GROUP;
        ExAcc.Trustee.ptstrName = "Everyone";

        if((dwError = GetSecurityInfo(hPipe, SE_KERNEL_OBJECT, DACL_SECURITY_INFORMATION, NULL, NULL, &pACL, NULL, NULL))!=ERROR_SUCCESS)
        {
            KbdcPrint(("GetSecurityInfo failed (code=%#x).\n", dwError));
            return EXIT_FAILURE;
        }

        if((dwError = SetEntriesInAcl(1, &ExAcc, pACL, &pACL))!=ERROR_SUCCESS)
        {
            KbdcPrint(("SetEntriesInAcl failed (code=%#x).\n", dwError));
            return EXIT_FAILURE;
        }

        if((dwError = SetSecurityInfo(hPipe, SE_KERNEL_OBJECT, DACL_SECURITY_INFORMATION, NULL, NULL, pACL, NULL))!=ERROR_SUCCESS)
        {
            KbdcPrint(("SetSecurityInfo failed (code=%#x).\n", dwError));
            return EXIT_FAILURE;
        }

        if(pACL)
        {
            LocalFree(pACL);
        }

        /*
            Idle until someone connects.
        */
        KbdcPrint(("Waiting for pipe %p to connect...\n", hPipe));

        if(!KbdcServPipeConnect(hPipe))
        {
            KbdcPrint(("KbdcConnectPipe failed (code=%#x).\n", GetLastError()));
            CloseHandle(hPipe);
        }

        if(WaitForSingleObject(l_State.hExitEvent, 0)==WAIT_OBJECT_0)
        {
            break;
        }

        if(WaitForSingleObject(l_State.hStopEvent, 0)==WAIT_OBJECT_0)
        {
            KbdcPrint(("Pausing pipe manager thread...\n"));

            KbdcServPipeDisconnectAll();

            /*
                go to sleep
            */
            SuspendThread(GetCurrentThread());

            KbdcPrint(("Resuming pipe manager thread...\n"));
        }

        KbdcPrint(("Pipe %p connected.\n", hPipe));
    }

    KbdcPrint(("Stopping pipe manager thread...\n"));

    KbdcServPipeDisconnectAll();

    KbdcPrint(("Pipe manager thread stopped.\n"));
    return 0;
}

VOID __WDECL KbdcServProcessPacket(PKBDCINPUTDATA pKid)
{
    LPKBDCSERVDEVICESTATE pDS = &l_State.DevState[pKid->DeviceType];

    /*
        translate scancodes into characters
        (English Keyboard Layout)
    */
    switch(pKid->MakeCode)
    {
        case 14:  /* BKSP */
            if(pKid->Flags&KIDF_BREAK)
            {/* released */
                ;
            }
            else if(pDS->CharBufferLength)
            {
                pDS->CharBufferLength--;
            }
            break;
        case 29:  /* LCTRL (1)/RCTRL (2) */
            if(pKid->Flags&KIDF_BREAK)
            {
                pDS->KeyState.CTRL&=~( (pKid->Flags&KIDF_E0) ? 0x2 : 0x1 );
            }
            else
            {
                pDS->KeyState.CTRL|= ( (pKid->Flags&KIDF_E0) ? 0x2 : 0x1 );
            }
            break;
        case 42:  /* LSHIFT (1) */
            if(pKid->Flags&KIDF_BREAK)
            {
                pDS->KeyState.SHIFT&=~0x1;
            }
            else
            {
                pDS->KeyState.SHIFT|= 0x1;
            }
            break;
        case 54:  /* RSHIFT (2) */
            if(pKid->Flags&KIDF_BREAK)
            {
                pDS->KeyState.SHIFT&=~0x2;
            }
            else
            {
                pDS->KeyState.SHIFT|= 0x2;
            }
            break;
        case 56:  /* LALT (1)/RALT (2) */
            if(pKid->Flags&KIDF_BREAK)
            {
                pDS->KeyState.ALT&=~( (pKid->Flags&KIDF_E0) ? 0x2 : 0x1 );
            }
            else
            {
                pDS->KeyState.ALT|= ( (pKid->Flags&KIDF_E0) ? 0x2 : 0x1 );
            }
            break;
        case 58:  /* CAPS LOCK */
            if(pKid->Flags&KIDF_BREAK)
            {
                ;
            }
            else
            {
                pDS->KeyState.CAPSLOCK = !pDS->KeyState.CAPSLOCK;
            }
            break;
        case 69:  /* NUM LOCK */
            if(pKid->Flags&KIDF_BREAK)
            {
                ;
            }
            else
            {
                pDS->KeyState.NUMLOCK = !pDS->KeyState.NUMLOCK;
            }
            break;
        case 70:  /* SCROLL LOCK */
            if(pKid->Flags&KIDF_BREAK)
            {
                ;
            }
            else
            {
                pDS->KeyState.SCROLLLOCK = !pDS->KeyState.SCROLLLOCK;
            }
            break;
        default:
        {
            BYTE ucChar = 0;

            if(pDS->KeyState.SHIFT)
            {/* shift */
                if(pKid->MakeCode<__ARRAYSIZE(l_ScanCodeToAsciiS))
                {
                    ucChar = l_ScanCodeToAsciiS[pKid->MakeCode];

                    if(pDS->KeyState.CAPSLOCK && ucChar>='A' && ucChar<='Z')
                    {
                        ucChar^= 0x20;  /* lowercase */
                    }
                }
            }
            else
            {/* normal */
                if(pKid->MakeCode<__ARRAYSIZE(l_ScanCodeToAsciiN))
                {
                    ucChar = l_ScanCodeToAsciiN[pKid->MakeCode];

                    if(pDS->KeyState.CAPSLOCK && ucChar>='a' && ucChar<='z')
                    {
                        ucChar^= 0x20;  /* uppercase */
                    }
                }
            }

            if(pKid->Flags&KIDF_BREAK)
            {/* released */
                ;
            }
            else if(ucChar)
            {/* pressed / repeat */
                pDS->CharBuffer[pDS->CharBufferLength++] = ucChar;

                if(ucChar=='\n')
                {
                    KbdcPrint(("KbdcServProcessPacket: %d = [%c] [%c] [%c] [%c] [%c] [%c]\n",
                        pKid->DeviceType,
                        pDS->KeyState.SHIFT ? '+' : ' ',
                        pDS->KeyState.CTRL ? '^' : ' ',
                        pDS->KeyState.ALT ? '!' : ' ',
                        pDS->KeyState.NUMLOCK ? '*' : ' ',
                        pDS->KeyState.CAPSLOCK ? '*' : ' ',
                        pDS->KeyState.SCROLLLOCK ? '*' : ' '));

                    KbdcServPipeBroadcastPacket(pDS);
                    pDS->CharBufferLength = 0;
                }
                else if(pDS->CharBufferLength>=__ARRAYSIZE(pDS->CharBuffer))
                {/* overflow, discard */
                    KbdcPrint(("KbdcServProcessPacket: Buffer for device %d full, discarding data.\n", pKid->DeviceType));
                    pDS->CharBufferLength = 0;
                }
            }
            break;
        }
    }
}

UINT __WDECL KbdcServMain(VOID)
{
    DWORD dwThreadId, dwRead, dwIdx, dwWait;
    HANDLE hAvailEvent, hFile, hThread, Pipe[KBDCSERV_PIPESIZE];
    KBDCINPUTDATA Kid[KBDCSERV_SIZE];
    OVERLAPPED Ovl;
    UINT uExitCode = EXIT_FAILURE;
    USHORT huDevType;

    /* stuff that cannot fail */
    InitializeCriticalSection(&l_State.PipeCS);
    BVSQueInit(&l_State.PipeQueue, Pipe, sizeof(Pipe));

    /* initialize pipe buffer prefix fields */
    for(huDevType = 0; huDevType<__ARRAYSIZE(l_State.DevState); huDevType++)
    {
        l_State.DevState[huDevType].DeviceType = huDevType;
    }

    if((hAvailEvent = CreateEvent(NULL, FALSE, FALSE, NULL))!=NULL)
    {
        if((hThread = CreateThread(NULL, 0, &KbdcServPipeManager, NULL, CREATE_SUSPENDED, &dwThreadId))!=NULL)
        {
            for(;;)
            {
                if((hFile = CreateFileA("\\\\.\\KbdCatch", GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL))!=INVALID_HANDLE_VALUE)
                {
                    ResumeThread(hThread);

                    for(;;)
                    {
                        Ovl.Offset = Ovl.OffsetHigh = 0;
                        Ovl.hEvent = hAvailEvent;

                        if(!ReadFile(hFile, Kid, sizeof(Kid), &dwRead, &Ovl))
                        {
                            HANDLE Events[] =
                            {
                                hAvailEvent,
                                l_State.hExitEvent,
                            };

                            if(GetLastError()!=ERROR_IO_PENDING)
                            {
                                KbdcPrint(("ReadFile failed (code=%#x).\n", GetLastError()));
                                break;
                            }

                            dwWait = WaitForMultipleObjects(__ARRAYSIZE(Events), Events, FALSE, INFINITE);

                            if(dwWait==WAIT_FAILED)
                            {
                                KbdcPrint(("WaitForMultipleObjects during ReadFile completion failed (code=%#x).\n", GetLastError()));
                                CancelIo(hFile);
                                break;
                            }
                            else if(dwWait==WAIT_OBJECT_0)
                            {/* ReadFile complete */
                                if(!GetOverlappedResult(hFile, &Ovl, &dwRead, FALSE))
                                {
                                    KbdcPrint(("GetOverlappedResult failed (code=%#x).\n", GetLastError()));
                                    break;
                                }
                            }
                            else if(dwWait==WAIT_OBJECT_0+1)
                            {/* hExitEvent signaled */
                                uExitCode = EXIT_SUCCESS;
                                CancelIo(hFile);
                                break;
                            }
                            else
                            {
                                KbdcPrint(("WaitForMultipleObjects returned unexpected value %#x (code=%#x).\n", dwWait, GetLastError()));
                                CancelIo(hFile);
                                break;
                            }
                        }

                        if(dwRead%sizeof(Kid[0]))
                        {
                            KbdcPrint(("Received unexpected data size %u (%u, %u, %u).\n", dwRead, dwRead/sizeof(Kid[0]), dwRead%sizeof(Kid[0]), sizeof(Kid[0])));
                        }

                        for(dwIdx = 0; dwIdx<dwRead/sizeof(Kid[0]); dwIdx++)
                        {
                            KbdcServProcessPacket(&Kid[dwIdx]);
                        }

                        if(WaitForSingleObject(l_State.hExitEvent, 0)==WAIT_OBJECT_0)
                        {
                            uExitCode = EXIT_SUCCESS;
                            break;
                        }
                    }

                    /*
                        disconnect all listeners
                        - hThread will be suspended
                        - hStopEvent is auto-reset
                    */
                    SetEvent(l_State.hStopEvent);

                    CloseHandle(hFile);
                    hFile = INVALID_HANDLE_VALUE;
                }
                else
                {
                    KbdcPrint(("Failed to open communication device (code=%#x), will try later.\n", GetLastError()));
                }

                if(WaitForSingleObject(l_State.hExitEvent, 60000)==WAIT_OBJECT_0)
                {
                    uExitCode = EXIT_SUCCESS;
                    break;
                }
            }

            /*
                ensure, that the thread exits
            */
            ResumeThread(hThread);
            SetEvent(l_State.hExitEvent);
            WaitForSingleObject(hThread, INFINITE);
            CloseHandle(hThread);
        }
        else
        {
            KbdcPrint(("Failed to create pipe thread (code=%#x).\n", GetLastError()));
        }

        CloseHandle(hAvailEvent);
    }
    else
    {
        KbdcPrint(("Failed to create avail event (code=%#x).\n", GetLastError()));
    }

    DeleteCriticalSection(&l_State.PipeCS);

    return uExitCode;
}

BOOL CALLBACK ServiceOnControlEx(DWORD dwControl, DWORD dwEventType, LPVOID lpEventData)
{
    switch(dwControl)
    {
        case SERVICE_CONTROL_SESSIONCHANGE:
        {
            PWTSSESSION_NOTIFICATION pWSN = (PWTSSESSION_NOTIFICATION)lpEventData;

            switch(dwEventType)
            {
                case WTS_REMOTE_CONNECT:
                case WTS_SESSION_LOCK:
                    l_State.SessionLocked = TRUE;
                    KbdcPrint(("ServiceOnControlEx: Session locked.\n"));
                    break;
                case WTS_REMOTE_DISCONNECT:
                case WTS_SESSION_UNLOCK:
                    l_State.SessionLocked = FALSE;
                    KbdcPrint(("ServiceOnControlEx: Session unlocked.\n"));
                    break;
            }
            break;
        }
        default:
            return FALSE;
    }

    return TRUE;
}

BOOL CALLBACK ServiceOnControl(DWORD dwType)
{
    SetEvent(l_State.hExitEvent);
    return TRUE;
}

BOOL CALLBACK ServiceMain(DWORD dwArgc, LPTSTR* lppszArgv)
{
    BOOL bResult = FALSE;

    if(!BvServiceParseMainteArgs(dwArgc, lppszArgv, &bResult))
    {
        return bResult;
    }

    BvServiceReportStatus(SERVICE_RUNNING, NO_ERROR, 0);

    if(!BvServiceActive())
    {
        SetConsoleCtrlHandler(&ServiceOnControl, TRUE);
    }

    if((l_State.hExitEvent = CreateEvent(NULL, TRUE, FALSE, NULL))!=NULL)
    {
        if((l_State.hStopEvent = CreateEvent(NULL, FALSE, FALSE, NULL))!=NULL)
        {
            bResult = KbdcServMain()==EXIT_SUCCESS;

            CloseHandle(l_State.hStopEvent);
            l_State.hStopEvent = NULL;
        }
        else
        {
            bResult = FALSE;
        }

        CloseHandle(l_State.hExitEvent);
        l_State.hExitEvent = NULL;
    }
    else
    {
        bResult = FALSE;
    }

    if(!BvServiceActive())
    {
        SetConsoleCtrlHandler(&ServiceOnControl, FALSE);
    }

    return bResult;
}
