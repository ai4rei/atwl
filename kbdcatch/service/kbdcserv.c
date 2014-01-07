#include <windows.h>
#include <accctrl.h>
#include <aclapi.h>

#include <btypes.h>
#include <bvargs.h>
#include <bvdebug.h>
#include <bvsque.h>

#include "../kbdctype.h"

#define KBDCSERV_NAME "KbdcServ"
#define KBDCSERV_DISP "Keyboard Catcher Service"
#define KBDCSERV_SIZE 100
#define KBDCSERV_PIPENAME "\\\\.\\pipe\\KbdcData"
#define KBDCSERV_PIPESIZE 10

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
    SERVICEDATA ServiceData;
    BOOLEAN ServiceMode;
    BOOLEAN ServiceCreate;
    BOOLEAN ServiceDelete;
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

VOID __WDECL KbdcServReportStatus(DWORD dwCurrentState, DWORD dwWin32ExitCode, DWORD dwWaitHint)
{
    static DWORD dwCheckPoint = 1;

    l_State.ServiceData.Status.dwCurrentState  = dwCurrentState;
    l_State.ServiceData.Status.dwWin32ExitCode = dwWin32ExitCode;
    l_State.ServiceData.Status.dwWaitHint      = dwWaitHint;

    /* accepted controls */
    switch(dwCurrentState)
    {
        case SERVICE_RUNNING:
        case SERVICE_PAUSED:
        case SERVICE_CONTINUE_PENDING:
        case SERVICE_PAUSE_PENDING:
            l_State.ServiceData.Status.dwControlsAccepted = SERVICE_ACCEPT_STOP|SERVICE_ACCEPT_SHUTDOWN;
            break;
        case SERVICE_START_PENDING:
        case SERVICE_STOP_PENDING:
        case SERVICE_STOPPED:
            l_State.ServiceData.Status.dwControlsAccepted = 0;
            break;
    }

    /* check point handling */
    switch(dwCurrentState)
    {
        case SERVICE_STOPPED:
        case SERVICE_RUNNING:
        case SERVICE_PAUSED:
            l_State.ServiceData.Status.dwCheckPoint = 0;
            break;
        case SERVICE_START_PENDING:
        case SERVICE_STOP_PENDING:
        case SERVICE_CONTINUE_PENDING:
        case SERVICE_PAUSE_PENDING:
            l_State.ServiceData.Status.dwCheckPoint = dwCheckPoint++;
            break;
    }

    SetServiceStatus(l_State.ServiceData.StatusHandle, &l_State.ServiceData.Status);
}

BOOL __WDECL KbdcConnectPipe(HANDLE hPipe)
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
                    l_State.hExitEvent,
                };

                dwWait = WaitForMultipleObjects(__ARRAYSIZE(Events), Events, FALSE, INFINITE);

                if(dwWait==WAIT_OBJECT_0+1 || dwWait==WAIT_ABANDONED_0+1)
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

            BVSQuePushTail(&l_State.PipeQueue, hPipe);

            LeaveCriticalSection(&l_State.PipeCS);

            bSuccess = TRUE;
            break;
        }

        CloseHandle(hEvent);
    }

    return bSuccess;
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

        if(!KbdcConnectPipe(hPipe))
        {
            KbdcPrint(("KbdcConnectPipe failed (code=%#x).\n", GetLastError()));
            CloseHandle(hPipe);
        }

        if(WaitForSingleObject(l_State.hExitEvent, 0)==WAIT_OBJECT_0)
        {
            break;
        }

        KbdcPrint(("Pipe %p connected.\n", hPipe));
    }

    KbdcPrint(("Stopping pipe manager thread...\n"));

    while(BVSQueLength(&l_State.PipeQueue))
    {
        HANDLE hPipe = BVSQuePopTail(&l_State.PipeQueue);

        FlushFileBuffers(hPipe);
        DisconnectNamedPipe(hPipe);
        CloseHandle(hPipe);
    }

    if(pSD)
    {
        LocalFree(pSD);
    }

    if(pACL)
    {
        LocalFree(pACL);
    }

    KbdcPrint(("Pipe manager thread stopped.\n"));
    return 0;
}

VOID __WDECL KbdcServBroadcastPacket(LPKBDCSERVDEVICESTATE pDS)
{
    size_t uIdx;

    EnterCriticalSection(&l_State.PipeCS);

    for(uIdx = 0; uIdx<BVSQueLength(&l_State.PipeQueue); uIdx++)
    {
        DWORD dwRead;
        HANDLE hPipe = BVSQueAt(&l_State.PipeQueue, uIdx)[0];
        OVERLAPPED Ovl = { 0, 0, 0, 0, NULL };

        if(!WriteFile(hPipe, &pDS->DeviceType, sizeof(pDS->DeviceType)+pDS->CharBufferLength, &dwRead, &Ovl))
        {
            if(GetLastError()!=ERROR_IO_PENDING)
            {
                KbdcPrint(("WriteFile in KbdcServBroadcastPacket failed (code=%#x).\n", GetLastError()));
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

VOID __WDECL KbdcServProcessPacket(PKBDCINPUTDATA pKid)
{
    LPKBDCSERVDEVICESTATE pDS = &l_State.DevState[pKid->DeviceType];

    /*
        translate scancodes into characters
        (English Keyboard Layout)
    */
    switch(pKid->MakeCode)
    {
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

                    KbdcServBroadcastPacket(pDS);
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
    UINT uExitCode = EXIT_FAILURE;
    HANDLE hAvailEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

    if(hAvailEvent)
    {
        HANDLE hFile = CreateFileA("\\\\.\\KbdCatch", GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);

        if(hFile!=INVALID_HANDLE_VALUE)
        {
            DWORD dwThreadId;
            HANDLE hThread = CreateThread(NULL, 0, &KbdcServPipeManager, NULL, CREATE_SUSPENDED, &dwThreadId);

            if(hThread)
            {
                DWORD dwRead, dwIdx;
                HANDLE Pipe[KBDCSERV_PIPESIZE];
                KBDCINPUTDATA Kid[KBDCSERV_SIZE];
                OVERLAPPED Ovl;
                USHORT huDevType;

                /* remaining stuff that cannot fail */
                InitializeCriticalSection(&l_State.PipeCS);
                BVSQueInit(&l_State.PipeQueue, Pipe, sizeof(Pipe));

                /* initialize pipe buffer prefix fields */
                for(huDevType = 0; huDevType<__ARRAYSIZE(l_State.DevState); huDevType++)
                {
                    l_State.DevState[huDevType].DeviceType = huDevType;
                }

                ResumeThread(hThread);
                KbdcServReportStatus(SERVICE_RUNNING, NO_ERROR, 0);

                for(;;)
                {
                    Ovl.Offset = Ovl.OffsetHigh = 0;
                    Ovl.hEvent = hAvailEvent;

                    if(!ReadFile(hFile, Kid, sizeof(Kid), &dwRead, &Ovl))
                    {
                        DWORD dwWait;
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
                            KbdcPrint(("WaitForMultipleObjects returned unexcepted value %#x (code=%#x).\n", dwWait, GetLastError()));
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

                SetEvent(l_State.hExitEvent);  /* ensure, that the thread exits */
                WaitForSingleObject(hThread, INFINITE);
                CloseHandle(hThread);

                DeleteCriticalSection(&l_State.PipeCS);
            }
            else
            {
                KbdcPrint(("Failed to create pipe thread (code=%#x).\n", GetLastError()));
            }

            CloseHandle(hFile);
        }
        else
        {
            KbdcPrint(("Failed to open communication device (code=%#x).\n", GetLastError()));
        }

        CloseHandle(hAvailEvent);
    }
    else
    {
        KbdcPrint(("Failed to create avail event (code=%#x).\n", GetLastError()));
    }

    return uExitCode;
}

BOOL CALLBACK KbdcServOnCtrl(DWORD dwCtrlType)
{
    if(l_State.ServiceMode)
    {
        KbdcServReportStatus(SERVICE_STOP_PENDING, NO_ERROR, 3000);
    }

    SetEvent(l_State.hExitEvent);
    return TRUE;
}

VOID CALLBACK KbdcServOnCtrlHan(DWORD dwControl)
{
    switch(dwControl)
    {
        case SERVICE_CONTROL_STOP:
            KbdcServOnCtrl(CTRL_CLOSE_EVENT);
            break;
        case SERVICE_CONTROL_INTERROGATE:
            KbdcServReportStatus(l_State.ServiceData.Status.dwCurrentState, NO_ERROR, l_State.ServiceData.Status.dwWaitHint);
            break;
        case SERVICE_CONTROL_SHUTDOWN:
            KbdcServOnCtrl(CTRL_SHUTDOWN_EVENT);
            break;
    }
}

VOID CALLBACK KbdcServStart(DWORD dwArgc, LPSTR* lppszArgv)
{
    l_State.ServiceData.StatusHandle = RegisterServiceCtrlHandlerA(KBDCSERV_NAME, &KbdcServOnCtrlHan);

    if(l_State.ServiceData.StatusHandle)
    {
        l_State.ServiceData.Status.dwServiceType             = SERVICE_WIN32_OWN_PROCESS;
        l_State.ServiceData.Status.dwServiceSpecificExitCode = 0;

        KbdcServReportStatus(SERVICE_START_PENDING, NO_ERROR, 1000);

        if(KbdcServMain()==EXIT_SUCCESS)
        {
            SetLastError(ERROR_SUCCESS);
        }
    }

    KbdcServReportStatus(SERVICE_STOPPED, GetLastError(), 0);
}

BOOL __WDECL KbdcServSCM(SC_HANDLE* lphSCM, DWORD dwDesiredAccess)
{
    lphSCM[0] = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT|dwDesiredAccess);

    return lphSCM[0]!=NULL;
}

BOOL __WDECL KbdcServCreate(LPCSTR lpszServName, LPCSTR lpszServDisp, LPCSTR lpszDepends, LPCSTR lpszUser, LPCSTR lpszPass)
{
    BOOL bSuccess = FALSE;
    SC_HANDLE hSCM;

    if(KbdcServSCM(&hSCM, SC_MANAGER_ALL_ACCESS))
    {
        CHAR szModuleName[MAX_PATH], szBinName[MAX_PATH+32];

        if(GetModuleFileNameA(NULL, szModuleName, __ARRAYSIZE(szModuleName)))
        {
            SC_HANDLE hServ;

            wsprintf(szBinName, "\"%s\" %cservice", szModuleName, BVARGS_SWITCH_CHARACTER);

            hServ = CreateServiceA(
                hSCM,
                lpszServName,
                lpszServDisp,
                SERVICE_ALL_ACCESS,
                SERVICE_WIN32_OWN_PROCESS,
                SERVICE_AUTO_START,
                SERVICE_ERROR_NORMAL,
                szBinName,
                NULL,
                NULL,
                lpszDepends,
                lpszUser,
                lpszPass
            );

            if(hServ)
            {
                bSuccess = TRUE;

                CloseServiceHandle(hServ);
            }
        }

        CloseServiceHandle(hSCM);
    }

    return bSuccess;
}

BOOL __WDECL KbdcServDelete(LPCSTR lpszServName)
{
    BOOL bSuccess = FALSE;
    SC_HANDLE hSCM;

    if(KbdcServSCM(&hSCM, SC_MANAGER_ALL_ACCESS))
    {
        SC_HANDLE hServ = OpenServiceA(hSCM, lpszServName, DELETE|SERVICE_STOP);

        if(hServ)
        {
            SERVICE_STATUS ServStatus;

            for(;;)
            {
                if(!ControlService(hServ, SERVICE_CONTROL_STOP, &ServStatus))
                {
                    if(GetLastError()!=ERROR_SERVICE_NOT_ACTIVE)
                    {
                        break;
                    }
                }

                if(DeleteService(hServ))
                {
                    bSuccess = TRUE;
                }

                break;
            }

            CloseServiceHandle(hServ);
        }

        CloseServiceHandle(hSCM);
    }

    return bSuccess;
}

VOID __WDECL KbdcServEnter(VOID)
{
    unsigned char ucBuffer[256];
    unsigned long luArgc;
    char** lppszArgv = (char**)ucBuffer;
    UINT uExitCode = EXIT_FAILURE;

    if(BVArgsSplitEx(GetCommandLineA(), &luArgc, &lppszArgv, sizeof(ucBuffer)))
    {
        BVARGSPARSESWITCH Bps[] =
        {
            { "service", BVAPS_TYPE_BOOL, &l_State.ServiceMode   },
            { "create",  BVAPS_TYPE_BOOL, &l_State.ServiceCreate },
            { "delete",  BVAPS_TYPE_BOOL, &l_State.ServiceDelete },
        };
        BVARGSPARSEINFO Bpi =
        {
            Bps,
            __ARRAYSIZE(Bps),
        };

        if(BVArgsParse(luArgc, lppszArgv, &Bpi))
        {
            if(l_State.ServiceMode)
            {
                l_State.hExitEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

                if(l_State.hExitEvent)
                {
                    SERVICE_TABLE_ENTRY ServiceDispatchTable[] =
                    {
                        { KBDCSERV_NAME, &KbdcServStart },
                        { NULL, NULL }
                    };

                    if(StartServiceCtrlDispatcherA(ServiceDispatchTable))
                    {
                        uExitCode = EXIT_SUCCESS;
                    }
                    else
                    {
                        KbdcPrint(("Failed to connect to SCM (code=%#x).\n", GetLastError()));
                    }

                    CloseHandle(l_State.hExitEvent);
                    l_State.hExitEvent = NULL;
                }
                else
                {
                    KbdcPrint(("Failed to create exit event (code=%#x).\n", GetLastError()));
                }
            }
            else if(l_State.ServiceCreate)
            {
                if(KbdcServCreate(KBDCSERV_NAME, KBDCSERV_DISP, "", "NT AUTHORITY\\LocalService", NULL))
                {
                    uExitCode = EXIT_SUCCESS;
                }
                else
                {
                    KbdcPrint(("Failed to create service (code=%#x).\n", GetLastError()));
                }
            }
            else if(l_State.ServiceDelete)
            {
                if(KbdcServDelete(KBDCSERV_NAME))
                {
                    uExitCode = EXIT_SUCCESS;
                }
                else
                {
                    KbdcPrint(("Failed to delete service (code=%#x).\n", GetLastError()));
                }
            }
            else
            {
                l_State.hExitEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

                if(l_State.hExitEvent)
                {
                    SetConsoleCtrlHandler(&KbdcServOnCtrl, TRUE);

                    uExitCode = KbdcServMain();

                    CloseHandle(l_State.hExitEvent);
                    l_State.hExitEvent = NULL;
                }
                else
                {
                    KbdcPrint(("Failed to create exit event (code=%#x).\n", GetLastError()));
                }
            }
        }
        else
        {
            KbdcPrint(("Invalid command line parameters.\n"));
        }
    }
    else
    {
        KbdcPrint(("Command line too long.\n"));
    }

    ExitProcess(uExitCode);
}
