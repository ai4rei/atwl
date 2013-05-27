#pragma once

#include <windows.h>

#pragma region IMPORTS

typedef int (PASCAL *PSETSOCKOPT) (__in SOCKET, __in int, __in int, __in const char *, __in int);
typedef SOCKET (PASCAL *PSOCKET) (__in int, __in int, __in int);
typedef HMODULE (WINAPI *PLOADLIBRARY)(LPCSTR);
typedef FARPROC (WINAPI *PGETPROCADDRESS)(HMODULE, LPCSTR);
typedef HMODULE (WINAPI *PGETMODULEHANDLEA)(LPCSTR);

#pragma endregion

SOCKET PASCAL mySocket(int af, int type, int protocol);
void mySocketEnd();