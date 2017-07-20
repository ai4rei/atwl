#include <stdio.h>
#include <string.h>

#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib,"ws2_32.lib")

#define MAX_SERVER_CLIENTS 64
#define TEST_PORT 12345
#define TOSTR_(X) (#X)
#define TOSTR(X) TOSTR_(X)

typedef struct CLIENTCONNECTION
{
    SOCKET hSock;
    size_t uInLength;
    size_t uOuLength;
    BYTE ucIn[128];
    BYTE ucOu[256];
}
CLIENTCONNECTION;

static HANDLE l_hExitEvent = NULL;

// Notes:
//
// - I/O buffers would normally be ring buffers (do not move data around)
// - Parser would be a separate function

static void RemoveConnection(const size_t uIdx, const char* const lpszTopic, CLIENTCONNECTION* lpClient, size_t* const lpuClientCount, int nError)
{
    if(ERROR_SUCCESS==nError)
    {
        printf("Client #%u: Disconnected (%u total).\n", uIdx, lpuClientCount[0]);
    }
    else
    {
        printf("Client #%u: Disconnected due to a %s error %d (%u total).\n", uIdx, lpszTopic, nError, lpuClientCount[0]);
    }

    lpuClientCount[0]--;

    closesocket(lpClient->hSock);
    lpClient->hSock = INVALID_SOCKET;
}

static void FindSocketEvents(HANDLE hEvent, CLIENTCONNECTION* const ahSlots, const size_t uElements, size_t* const lpuClientCount)
{
    size_t uIdx;

    for(uIdx = 0U; uIdx<uElements; uIdx++)
    {
        CLIENTCONNECTION* lpClient = &ahSlots[uIdx];
        WSANETWORKEVENTS Ne;

        if(lpClient->hSock==INVALID_SOCKET)
        {
            continue;
        }

        if(ERROR_SUCCESS!=WSAEnumNetworkEvents(lpClient->hSock, hEvent, &Ne))
        {
            printf("Client #%u: Error enumerating network events (error %u).\n", uIdx, WSAGetLastError());
            continue;
        }

        // Process: Closure
        if(Ne.lNetworkEvents&FD_CLOSE)
        {
            RemoveConnection(uIdx, "close", lpClient, lpuClientCount, Ne.iErrorCode[FD_CLOSE_BIT]);
            continue;
        }

        // Process: Send previously unsent data
        if(Ne.lNetworkEvents&FD_WRITE)
        {
            if(ERROR_SUCCESS==Ne.iErrorCode[FD_WRITE_BIT])
            {
                if(lpClient->uOuLength)
                {// only if there is unsent data
                    DWORD dwSent = 0U;
                    WSABUF Buf = { lpClient->uOuLength, (char*)lpClient->ucOu };

                    if(ERROR_SUCCESS==WSASend(lpClient->hSock, &Buf, 1, &dwSent, 0U, NULL, NULL))
                    {
                        lpClient->uOuLength-= dwSent;

                        if(lpClient->uOuLength)
                        {
                            memmove(&lpClient->ucOu[0], &lpClient->ucOu[dwSent], lpClient->uOuLength);
                        }
                    }
                    else
                    if(WSAGetLastError()==WSAEWOULDBLOCK)
                    {// blocks of wood are fine
                        ;
                    }
                    else
                    {
                        RemoveConnection(uIdx, "send", lpClient, lpuClientCount, WSAGetLastError());
                        continue;
                    }
                }
            }
            else
            {
                RemoveConnection(uIdx, "write", lpClient, lpuClientCount, Ne.iErrorCode[FD_WRITE_BIT]);
                continue;
            }
        }

        // Process: Read new data
        if(Ne.lNetworkEvents&FD_READ)
        {
            if(ERROR_SUCCESS==Ne.iErrorCode[FD_READ_BIT])
            {
                const size_t uInAvail = sizeof(lpClient->ucIn)-lpClient->uInLength;

                if(uInAvail)
                {// only if there is buffer available
                    DWORD dwRead = 0U, dwFlags = 0U;
                    WSABUF Buf = { sizeof(lpClient->ucIn)-lpClient->uInLength, (char*)&lpClient->ucIn[lpClient->uInLength] };

                    if(ERROR_SUCCESS==WSARecv(lpClient->hSock, &Buf, 1, &dwRead, &dwFlags, NULL, NULL))
                    {
                        if(dwRead)
                        {
                            lpClient->uInLength+= dwRead;
                        }
                        else
                        {// you are done sending, so are we
                            shutdown(lpClient->hSock, SD_SEND);
                        }
                    }
                    else
                    if(WSAGetLastError()==WSAEWOULDBLOCK)
                    {// blocks of wood are fine
                        ;
                    }
                    else
                    {
                        RemoveConnection(uIdx, "recv", lpClient, lpuClientCount, WSAGetLastError());
                        continue;
                    }
                }
            }
            else
            {
                RemoveConnection(uIdx, "read", lpClient, lpuClientCount, Ne.iErrorCode[FD_READ_BIT]);
                continue;
            }
        }

        // Process: Parse buffered data
        while(lpClient->uInLength)
        {
            // Example: Process CRLF terminated data (line-wise)
            unsigned char* lpucCRLF = (unsigned char*)memchr(lpClient->ucIn, '\r', lpClient->uInLength-1U);  // CR, -1 because we expect the last byte to be LF

            if(lpucCRLF)
            {
                if(lpucCRLF[1]=='\n')
                {// a fine line, send it back
                    const size_t uOuAvail = sizeof(lpClient->ucOu)-lpClient->uOuLength;
                    const size_t uLineLength = (lpucCRLF-lpClient->ucIn)+2U;

                    if(uOuAvail>=uLineLength)
                    {// it fits
                        memcpy(&lpClient->ucOu[lpClient->uOuLength], &lpClient->ucIn[0], uLineLength);
                        lpClient->uOuLength+= uLineLength;

                        printf("Client #%u: [%.*s]\n", uIdx, uLineLength-2, lpClient->ucIn);
                    }
                    else
                    {
                        printf("Client #%u: Exceeded outgoing buffer, data lost.\n", uIdx);
                    }

                    lpClient->uInLength-= uLineLength;

                    if(lpClient->uInLength)
                    {
                        memmove(&lpClient->ucIn[0], &lpClient->ucIn[uLineLength], lpClient->uInLength);
                    }
                }
                else
                {
                    const size_t uLineLength = lpucCRLF-lpClient->ucIn+1U;

                    printf("Client #%u: Bad data received, discarding.\n", uIdx);

                    lpClient->uInLength-= uLineLength;

                    if(lpClient->uInLength)
                    {
                        memmove(&lpClient->ucIn[0], &lpClient->ucIn[uLineLength], lpClient->uInLength);
                    }
                }
            }
            else
            {// done parsing
                break;
            }
        }
        // Parser End

        if(lpClient->uInLength==sizeof(lpClient->ucIn))
        {// incoming buffer full after parsing -> non-recoverable error (as long we use static buffers)
            printf("Client #%u: Exceeded incoming buffer, closing connection.\n", uIdx);

            RemoveConnection(uIdx, "", lpClient, lpuClientCount, 0);
            continue;
        }

        // Process: Send new unsent data
        if(lpClient->uOuLength)
        {// only if there is unsent data
            DWORD dwSent = 0U;
            WSABUF Buf = { lpClient->uOuLength, (char*)lpClient->ucOu };

            if(ERROR_SUCCESS==WSASend(lpClient->hSock, &Buf, 1, &dwSent, 0U, NULL, NULL))
            {
                lpClient->uOuLength-= dwSent;

                if(lpClient->uOuLength)
                {
                    memmove(&lpClient->ucOu[0], &lpClient->ucOu[dwSent], lpClient->uOuLength);
                }
            }
            else
            if(WSAGetLastError()==WSAEWOULDBLOCK)
            {// blocks of wood are fine
                ;
            }
            else
            {
                RemoveConnection(uIdx, "send", lpClient, lpuClientCount, WSAGetLastError());
                continue;
            }
        }
    }
}

static size_t FindNextSocketSlot(CLIENTCONNECTION* const ahSlots, const size_t uElements, const size_t uLastIdx)
{
    size_t uIdx;
    size_t uEnd;

    if(uLastIdx && uLastIdx<uElements)
    {
        uEnd = uLastIdx+1U;

        for(uIdx = uEnd; uIdx<uElements; uIdx++)
        {
            if(ahSlots[uIdx].hSock==INVALID_SOCKET)
            {// free
                return uIdx;
            }
        }
    }
    else
    {
        uEnd = uElements;
    }

    for(uIdx = 0U; uIdx<=uEnd; uIdx++)
    {
        if(ahSlots[uIdx].hSock==INVALID_SOCKET)
        {// free
            return uIdx;
        }
    }

    return uElements;  // not found
}

static BOOL CALLBACK CtrlBreak(DWORD dwCtrlType)
{
    SetEvent(l_hExitEvent);

    return TRUE;
}

void main(void)
{
    printf("Starting up...\n");

    l_hExitEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    if(l_hExitEvent)
    {
        HANDLE hAcceptEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

        SetConsoleCtrlHandler(&CtrlBreak, TRUE);

        if(hAcceptEvent)
        {
            HANDLE hInOutEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

            if(hInOutEvent)
            {
                WSADATA Wd;

                if(ERROR_SUCCESS==WSAStartup(MAKEWORD(2,2), &Wd))
                {
                    struct addrinfo* lpai = NULL;

                    if(ERROR_SUCCESS==getaddrinfo("0.0.0.0", TOSTR(TEST_PORT), NULL, &lpai))
                    {
                        SOCKET hServerSock = socket(lpai->ai_family, SOCK_STREAM, IPPROTO_TCP);

                        if(hServerSock!=INVALID_SOCKET)
                        {
                            if(ERROR_SUCCESS==bind(hServerSock, lpai->ai_addr, lpai->ai_addrlen))
                            {
                                if(ERROR_SUCCESS==listen(hServerSock, SOMAXCONN))
                                {
                                    if(ERROR_SUCCESS==WSAEventSelect(hServerSock, hAcceptEvent, FD_ACCEPT))  // non-blocking
                                    {
                                        size_t uIdx = 0U, uClientCount = 0U;
                                        HANDLE Events[3];
                                        CLIENTCONNECTION ahClients[MAX_SERVER_CLIENTS] = { 0 };

                                        Events[0] = l_hExitEvent;
                                        Events[1] = hAcceptEvent;
                                        Events[2] = hInOutEvent;

                                        // initialize array
                                        for(uIdx = 0U; uIdx<_ARRAYSIZE(ahClients); uIdx++)
                                        {
                                            ahClients[uIdx].hSock = INVALID_SOCKET;
                                        }

                                        printf("Listening for incoming connections.\n");

                                        for(;;)
                                        {
                                            DWORD dwWait = WaitForMultipleObjects(_ARRAYSIZE(Events), Events, FALSE, INFINITE);

                                            if(dwWait==WAIT_OBJECT_0+0U)
                                            {// server is quiting
                                                break;
                                            }
                                            else
                                            if(dwWait==WAIT_OBJECT_0+1U)
                                            {// new connection
                                                uIdx = FindNextSocketSlot(ahClients, _ARRAYSIZE(ahClients), uIdx);

                                                if(uIdx<_ARRAYSIZE(ahClients))
                                                {// accept only if there is room
                                                    SOCKET hSock = accept(hServerSock, NULL, NULL);

                                                    if(hSock!=INVALID_SOCKET)
                                                    {
                                                        if(ERROR_SUCCESS==WSAEventSelect(hSock, hInOutEvent, FD_READ|FD_WRITE|FD_CLOSE))
                                                        {
                                                            ahClients[uIdx].hSock = hSock;
                                                            ahClients[uIdx].uInLength = 0U;
                                                            ahClients[uIdx].uOuLength = 0U;

                                                            uClientCount++;

                                                            printf("Client #%u: Connected (%u total).\n", uIdx, uClientCount);
                                                        }
                                                        else
                                                        {
                                                            printf("Failed to change event select on socket (error %u).\n", WSAGetLastError());
                                                            closesocket(hSock);
                                                        }
                                                    }
                                                    else
                                                    if(WSAGetLastError()==WSAEWOULDBLOCK)
                                                    {// this can happen and is completely normal, try later
                                                        ;
                                                    }
                                                    else
                                                    {
                                                        printf("Failed to accept socket (error %u).\n", WSAGetLastError());
                                                    }
                                                }

                                                continue;
                                            }
                                            else
                                            if(dwWait==WAIT_OBJECT_0+2U)
                                            {// i/o available
                                                FindSocketEvents(hInOutEvent, ahClients, _ARRAYSIZE(ahClients), &uClientCount);

                                                continue;
                                            }

                                            printf("Unexpected wait result %u, exiting.\n", dwWait);
                                            break;
                                        }

                                        printf("Shutting down...\n");

                                        // kick-out clients
                                        for(uIdx = 0U; uIdx<_ARRAYSIZE(ahClients); uIdx++)
                                        {
                                            if(ahClients[uIdx].hSock!=INVALID_SOCKET)
                                            {
                                                shutdown(ahClients[uIdx].hSock, SD_SEND);
                                                closesocket(ahClients[uIdx].hSock);
                                                ahClients[uIdx].hSock = INVALID_SOCKET;
                                            }
                                        }
                                    }
                                }
                            }

                            closesocket(hServerSock);
                        }

                        freeaddrinfo(lpai);
                    }

                    WSACleanup();
                }

                CloseHandle(hInOutEvent);
            }

            CloseHandle(hAcceptEvent);
        }

        SetConsoleCtrlHandler(&CtrlBreak, FALSE);

        CloseHandle(l_hExitEvent);
    }

    printf("Done.\n");
}
