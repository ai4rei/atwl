// -----------------------------------------------------------------
// NOTE: this is trimmed-down version with dependencies only
// Must not be used with other software than RO Open Setup.
// (c) 2009-2010 Ai4rei/AN
// -----------------------------------------------------------------

#include "btypes.h"
#include "cpenum.h"

#include <stdlib.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

static struct CodePageEnumInfo* lpCurrentCpe = NULL;
static BOOL (WINAPI *_GetCPInfoEx)(UINT CodePage, DWORD dwFlags, LPCPINFOEX lpCPInfoEx) = NULL;
static CRITICAL_SECTION cs;

static int __cdecl CPE_P_SortCodePages(const void* lpItemA, const void* lpItemB)
{
    struct CodePageEnumInfoEntry* lpEntryA = (struct CodePageEnumInfoEntry*)lpItemA;
    struct CodePageEnumInfoEntry* lpEntryB = (struct CodePageEnumInfoEntry*)lpItemB;

    return lpEntryA->luCodePage-lpEntryB->luCodePage;
}

static BOOL CALLBACK CPE_P_EnumCodePagesProc(char* lpszCodePageString)
{
    struct CodePageEnumInfoEntry* lpEntry;

    if(lpCurrentCpe->luEntries>=MAX_CODEPAGE_ENTRIES)
    {
        return FALSE;
    }

    lpEntry = &lpCurrentCpe->Entries[(lpCurrentCpe->luEntries)++];
    lpEntry->luCodePage = strtoul(lpszCodePageString, NULL, 10);
    lpEntry->szCodePageName[0] = 0;

    if(_GetCPInfoEx)
    {
        CPINFOEX Info;

        if(_GetCPInfoEx(lpEntry->luCodePage, 0, &Info))
        {
            lstrcpynA(lpEntry->szCodePageName, Info.CodePageName, sizeof(lpEntry->szCodePageName));
        }
    }

    if(!lpEntry->szCodePageName[0])
    {// still nameless? emulate somewhat
        wsprintfA(lpEntry->szCodePageName, "%lu  (Unknown)", lpEntry->luCodePage);
    }

    return TRUE;
}

bool __stdcall CPE_EnumCodePagesEx(struct CodePageEnumInfo* lpCpe, int nOptions)
{
    bool bSuccess = false;

    EnterCriticalSection(&cs);

    lpCurrentCpe = lpCpe;
    lpCurrentCpe->luEntries = 0;

    if(EnumSystemCodePages(&CPE_P_EnumCodePagesProc, (nOptions&CPE_FLAG_INSTALLEDONLY) ? CP_INSTALLED : CP_SUPPORTED))
    {
        bSuccess = true;

        if(nOptions&CPE_FLAG_SORTID)
        {
            qsort(&lpCpe->Entries[0], lpCpe->luEntries, sizeof(lpCpe->Entries[0]), &CPE_P_SortCodePages);
        }
    }
    lpCurrentCpe = NULL;

    LeaveCriticalSection(&cs);

    return bSuccess;
}

void __stdcall CPE_Init(void)
{
    InitializeCriticalSection(&cs);
    _GetCPInfoEx = (void*)GetProcAddress(GetModuleHandle("kernel32.dll"), "GetCPInfoExA");
}

void __stdcall CPE_Quit(void)
{
    DeleteCriticalSection(&cs);
}
