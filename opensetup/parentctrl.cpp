// -----------------------------------------------------------------
// RagnarokOnline OpenSetup
// (c) 2010+ Ai4rei/AN
// See doc/license.txt for details.
//
// -----------------------------------------------------------------

#include <windows.h>
#include <tlhelp32.h>

#include "parentctrl.h"

CParentCtrl::CParentCtrl()
{
    HMODULE hDll = GetModuleHandle("kernel32.dll");
    LPFNCREATETOOLHELP32SNAPSHOT CreateToolhelp32Snapshot = (LPFNCREATETOOLHELP32SNAPSHOT)GetProcAddress(hDll, "CreateToolhelp32Snapshot");
    LPFNPROCESS32FIRSTNEXT Process32First = (LPFNPROCESS32FIRSTNEXT)GetProcAddress(hDll, "Process32First");
    LPFNPROCESS32FIRSTNEXT Process32Next = (LPFNPROCESS32FIRSTNEXT)GetProcAddress(hDll, "Process32Next");

    m_hParent = NULL;

    if(CreateToolhelp32Snapshot && Process32First && Process32Next)
    {
        HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

        if(hSnap!=INVALID_HANDLE_VALUE)
        {
            BOOL bExists;
            DWORD dwPID = GetCurrentProcessId();
            PROCESSENTRY32 PE = { sizeof(PE) };

            for(bExists = Process32First(hSnap, &PE); bExists; bExists = Process32Next(hSnap, &PE))
            {
                if(PE.th32ProcessID==dwPID)
                {
                    m_hParent = OpenProcess(PROCESS_TERMINATE, FALSE, PE.th32ParentProcessID);
                    break;
                }
            }

            CloseHandle(hSnap);
        }
    }
}

CParentCtrl::~CParentCtrl()
{
    if(m_hParent)
    {
        CloseHandle(m_hParent);
        m_hParent = NULL;
    }
}

bool CParentCtrl::IsAvail()
{
    return m_hParent!=NULL;
}

bool CParentCtrl::Kill()
{
    return TerminateProcess(m_hParent, EXIT_FAILURE)!=FALSE;
}
