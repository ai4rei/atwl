#define WIN32_LEAN_AND_MEAN
#include <windows.h>

/* PATCH: Enable GRF archives over 2GiB.
*/
DWORD WINAPI _SetFilePointer(HANDLE hFile, LONG lDistanceToMove, PLONG lpDistanceToMoveHigh, DWORD dwMoveMethod)
{
    if(lpDistanceToMoveHigh==NULL && lDistanceToMove<0 && dwMoveMethod==FILE_BEGIN)
    {
        LONG lHigh = 0;

        return SetFilePointer(hFile, lDistanceToMove, &lHigh, dwMoveMethod);
    }

    return SetFilePointer(hFile, lDistanceToMove, lpDistanceToMoveHigh, dwMoveMethod);
}

/* PATCH: Allow reading of 'foldered' effects. By default all
   resource reads are done from data\\texture\\effect, even though
   the resources should be loaded from the .str file path.
*/
char const* WINAPI PathFromStrPath(char const* const lpszStrPath)
{
    static struct DELPHISTR
    {
        DWORD dwUnk;
        DWORD dwLength;
        char szBuffer[MAX_PATH];
    }
    DS = { 0 };

    strncpy(DS.szBuffer, lpszStrPath, RTL_NUMBER_OF(DS.szBuffer));
    DS.szBuffer[RTL_NUMBER_OF(DS.szBuffer)-1] = '\0';
    strrchr(DS.szBuffer, '\\')[1] = '\0';
    DS.dwUnk = 0xFFFFFFFF;
    DS.dwLength = strlen(DS.szBuffer);

    return &DS.szBuffer[0];
}

__declspec(naked) void PathFromStrPathCB(void)
{
    // EDX contains pointer to "data\\texture\\effect\\"
    // ESP+0x1B0+0x4 contains pointer to current .str file
    // If the patch was successful, then 0x405178 is the stolen CALL
    // destination.
    __asm
    {
        PUSH    EAX
        PUSH    ECX

        MOV     EAX,DWORD PTR SS:[ESP+0x1B0+0x4+0x8]    ; +Offset+ReturnAddress+PushX2
        PUSH    EAX
        CALL    PathFromStrPath
        MOV     EDX,EAX

        POP     ECX
        POP     EAX

        PUSH    0x405178
        RETN
    }
}

static BOOL InstallHotFix(void)
{
#pragma pack(push,1)
    struct MATCH
    {
        BYTE A1; LONG A2;  // CALL  <->
        WORD B1; BYTE B2;  // MOV   EDX,DWORD PTR SS:[EBP-10]
        WORD C;            // MOV   ECX,ESI
        WORD D;            // MOV   EAX,EBX
        BYTE E1; LONG E2;  // CALL  <+>
        BYTE F1; BYTE F2;  // JMP   <+>
    };
#pragma pack(pop)
    struct MATCH* const lpMatch = (struct MATCH*)0x485F6C;
    struct MATCH const Needle =
    {
        0xE8, 0xFFF7F207,
        0x558B, 0xF0,
        0xCE8B,
        0xC38B,
        0xE8, 0x0000009F,
        0xEB, 0x1C,
    };

    if(memcmp(&Needle, lpMatch, sizeof(Needle))==0)
    {
        DWORD dwOldProtect;

        if(VirtualProtect(lpMatch, sizeof(lpMatch[0]), PAGE_READWRITE, &dwOldProtect))
        {
            __asm
            {
                MOV     EDI,lpMatch
                MOV     EAX,OFFSET PathFromStrPathCB        ; get function offset
                SUB     EAX,EDI                             ; subtract source
                SUB     EAX,5U                              ; subtract commands
                MOV     DWORD PTR DS:[EDI+1],EAX            ; CALL offset
            }

            VirtualProtect(lpMatch, sizeof(lpMatch[0]), dwOldProtect, &dwOldProtect);
            return TRUE;
        }
    }

    return FALSE;
}

BOOL CALLBACK DllMain(HINSTANCE hDll, DWORD dwReason, LPVOID lpReserved)
{
    if(dwReason==DLL_PROCESS_ATTACH)
    {
        return InstallHotFix();
    }

    return TRUE;
}
