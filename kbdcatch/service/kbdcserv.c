#include <windows.h>

#include <btypes.h>
#include <bvargs.h>
#include <bvdebug.h>

#include "../kbdctype.h"

#define KBDCSERV_NAME "KbdcServ"
#define KBDCSERV_DISP "Keyboard Catcher Service"
#define KBDCSERV_SIZE 100

#ifdef _DEBUG
    #define KbdcPrint(_x_) BvDbgPrintf _x_
#else  /* _DEBUG */
    #define KbdcPrint(_x_)
#endif  /* _DEBUG */

typedef struct _SERVICEDATA
{
    SERVICE_STATUS Status;
    SERVICE_STATUS_HANDLE StatusHandle;
}
SERVICEDATA,* PSERVICEDATA;

typedef struct _KBDCSERVSTATE
{
    HANDLE hExitEvent;
    SERVICEDATA ServiceData;
    BOOLEAN ServiceMode;
    BOOLEAN ServiceCreate;
    BOOLEAN ServiceDelete;
}
KBDCSERVSTATE,* PKBDCSERVSTATE;

KBDCSERVSTATE l_State = { 0 };

VOID __WDECL KbdcServReportStatus(DWORD dwCurrentState, DWORD dwWin32ExitCode, DWORD dwWaitHint)
{
    static DWORD dwCheckPoint = 1;

    l_State.ServiceData.Status.dwCurrentState  = dwCurrentState;
    l_State.ServiceData.Status.dwWin32ExitCode = dwWin32ExitCode;
    l_State.ServiceData.Status.dwWaitHint      = dwWaitHint;

    // accepted controls
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

    // check point handling
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

VOID __WDECL KbdcServProcessPacket(PKBDCINPUTDATA pKid)
{
    KbdcPrint(("KbdcServProcessPacket: %u %u %u %u\n", pKid->DeviceType, pKid->MakeCode, pKid->Flags, pKid->Reserved));
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
            DWORD dwRead, dwIdx;
            KBDCINPUTDATA Kid[KBDCSERV_SIZE];
            OVERLAPPED Ovl;

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
                        KbdcPrint(("WaitForMultipleObjects failed (code=%#x).\n", GetLastError()));
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
