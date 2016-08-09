#include <stdio.h>
#include <wchar.h>
#include <windows.h>
#include <ntsecapi.h>
#include <tchar.h>

#define C2TN(X) case X: return _T(#X)

static LPCTSTR __stdcall LogonTypeToName(ULONG ulLogonType)
{
	switch(ulLogonType)
	{
		C2TN(Interactive);
		C2TN(Network);
		C2TN(Batch);
		C2TN(Service);
		C2TN(Proxy);
		C2TN(Unlock);
		C2TN(NetworkCleartext);
		C2TN(NewCredentials);
		C2TN(RemoteInteractive);
		C2TN(CachedInteractive);
		C2TN(CachedRemoteInteractive);
		C2TN(CachedUnlock);
	}

	return _T("<unknown>");
}

INT __cdecl _tmain(INT nArgc, LPTSTR* lppszArgv)
{
	NTSTATUS Status;
	ULONG ulSessionCount;
	PLUID lpSessionList;

	Status = LsaEnumerateLogonSessions(&ulSessionCount, &lpSessionList);

	if(Status==0)
	{
		_tprintf(_T("LUID\tDOMAIN\\USER\tSESSION\tTYPE\tPACKAGE\n"));

		for(ULONG ulIdx = 0; ulIdx<ulSessionCount; ulIdx++)
		{
			PLUID lpIdxSession = &lpSessionList[ulIdx];
			PSECURITY_LOGON_SESSION_DATA lpLsd;

			Status = LsaGetLogonSessionData(lpIdxSession, &lpLsd);

			if(Status==0)
			{
				if(lpLsd)
				{
					_tprintf(_T("%x:%x\t%s\\%s\t%u\t%s\t%s\n"), lpIdxSession->HighPart, lpIdxSession->LowPart, lpLsd->LogonDomain.Buffer, lpLsd->UserName.Buffer, lpLsd->Session, LogonTypeToName(lpLsd->LogonType), lpLsd->AuthenticationPackage.Buffer);

					LsaFreeReturnBuffer(lpLsd);
					lpLsd = NULL;
				}
				else
				{
					_tprintf(_T("%x:%x\t\t\t\t\n\n"), lpIdxSession->HighPart, lpIdxSession->LowPart);
				}

				if(lpIdxSession->HighPart==0x0 && lpIdxSession->LowPart==0x3e7)
				{
					//_tprintf(_T("^ LocalSystem\n"));
				}
			}
			else
			{
				_tprintf(_T("LsaGetLogonSessionData failed (status=%d).\n"), Status);
			}
		}

		LsaFreeReturnBuffer(lpSessionList);
		lpSessionList = NULL;
	}
	else
	{
		_tprintf(_T("LsaEnumerateLogonSessions failed (status=%d).\n"), Status);
	}

	return 0;
}
