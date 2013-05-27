#include "InjectedCode.h"

#pragma optimize( "", off )
#pragma code_seg(".inject")

#pragma pack(push, 1)

void __declspec(naked) mySocketEnd()
{
	__asm
	{
		PUSH 0xDEADBEEF
	}
};

SOCKET PASCAL mySocket(int af, int type, int protocol)
{
	UINT32 uDeadBeefCounter = 0;

	unsigned long luEndAddr = 0;
	unsigned long luDeadBeef = 0;
	unsigned long luFnAddr = 0;
	unsigned long luStrAddr = 0;

	PLOADLIBRARY pfnLoadLibrary = NULL;
	PGETPROCADDRESS pfnGetProcAddress = NULL;
	PGETMODULEHANDLEA pfnGetModuleHandleA = NULL;
	PSETSOCKOPT pfnSetSockOpt = NULL;
	PSOCKET pfnSocket = NULL;

	char *str_ws2_32 = NULL;
	char *str_setsockopt = NULL;
	char *str_socket = NULL;

	int flag = 1;
	SOCKET r;

	// Get EIP.
	__asm{
		call l_fake
		l_fake:
		pop luEndAddr
	}

	while(uDeadBeefCounter < 2)
	{
		luDeadBeef = (DWORD)(*(DWORD *)(++luEndAddr));

		if(luDeadBeef == 0xDEADBEEF)
			uDeadBeefCounter++;
	}

	luFnAddr = *((unsigned long *)(luEndAddr+4));
	pfnLoadLibrary = (PLOADLIBRARY)(*((unsigned long *)(luFnAddr)));

	luFnAddr = *((unsigned long *)(luEndAddr+8));
	pfnGetProcAddress = (PGETPROCADDRESS)(*((unsigned long *)(luFnAddr)));

	luFnAddr = *((unsigned long *)(luEndAddr+12));
	pfnGetModuleHandleA = (PGETMODULEHANDLEA)(*((unsigned long *)(luFnAddr)));
	
	luStrAddr = luEndAddr + 16;

	str_ws2_32 = (char *)luStrAddr;

	while(*((char *)(luStrAddr)) != '\0')
		luStrAddr++;

	str_setsockopt = (char *)(++luStrAddr);

	while(*((char *)(luStrAddr)) != '\0')
		luStrAddr++;

	str_socket = (char *)(++luStrAddr);

	pfnSocket = (PSOCKET)pfnGetProcAddress(pfnGetModuleHandleA(str_ws2_32), str_socket);
	pfnSetSockOpt = (PSETSOCKOPT)pfnGetProcAddress(pfnGetModuleHandleA(str_ws2_32), str_setsockopt);

	// Now, all functions are available, change socket option.

	r = pfnSocket(af, type, protocol);
	if(r != INVALID_SOCKET)
	{
		pfnSetSockOpt(r, IPPROTO_TCP, TCP_NODELAY, (char *)&flag, sizeof(int));
	}

	return r;
}

#pragma pack(pop)

#pragma code_seg()
#pragma optimize( "", on )