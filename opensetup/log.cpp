// -----------------------------------------------------------------
// RagnarokOnline OpenSetup
// (c) 2010+ Ai4rei/AN
// See doc/license.txt for details.
//
// -----------------------------------------------------------------

#include <windows.h>
#ifndef WITHOUT_INTEGRITY
#include <integrity.h>
#endif

#include "opensetup.h"
#include "resource.h"
#include "config.h"
#include "log.h"

// global instance
CLog g_Log;

const char* __stdcall CLog::P_Platform2Name(DWORD dwPlatformId)
{
    switch(dwPlatformId)
    {
        case VER_PLATFORM_WIN32s:        return "Win32s";
        case VER_PLATFORM_WIN32_WINDOWS: return "Windows";
        case VER_PLATFORM_WIN32_NT:      return "Windows NT";
    }

    DebugBreakHere();
    return "";
}

#ifndef WITHOUT_INTEGRITY
const char* __stdcall CLog::P_Integrity2Name(int nIntegrityLevel)
{
    switch(nIntegrityLevel)
    {
        case WNT_INTEGRITY_LEVEL_UNTRUSTED:  return "Untrusted";
        case WNT_INTEGRITY_LEVEL_LOW:        return "Low";
        case WNT_INTEGRITY_LEVEL_MEDIUM:     return "Medium";
        case WNT_INTEGRITY_LEVEL_MEDIUMHIGH: return "MediumHigh";
        case WNT_INTEGRITY_LEVEL_HIGH:       return "High";
        case WNT_INTEGRITY_LEVEL_SYSTEM:     return "System";
        case WNT_INTEGRITY_LEVEL_PROTECTED:  return "Protected";
    }

    DebugBreakHere();
    return "";
}
#endif

const char* __stdcall CLog::P_Type2Name(LOGENTRYTYPE nType)
{
    switch(nType)
    {
        case LET_NONE: return "";
        case LET_INFO: return "[Info] ";
        case LET_WARN: return "[Warning] ";
        case LET_FAIL: return "[Error] ";
    }

    return "<unknown>";
}

void __stdcall CLog::P_EntryInsert(LOGENTRYTYPE nType, const char* lpszMessage)
{
    LOGENTRY LogEntry;

    OutputDebugStringA(lpszMessage);

    while(m_luLimit && m_luLimit<m_LogQueue.size())
    {// log depth limit
        P_EntryDeleteFirst();
    }

    for(;;)
    {
        try
        {
            // copy message
            LogEntry.lpszMessage = new char[strlen(lpszMessage)+1];
            strcpy(LogEntry.lpszMessage, lpszMessage);
            break;
        }
        catch(...)
        {
            // destroy oldest entry if there is not enough memory
            P_EntryDeleteFirst();
        }
    }

    // set timestamp
    GetLocalTime(&LogEntry.stTimeStamp);

    // remaining data
    LogEntry.nType = nType;
    LogEntry.uLevel = m_uLevel-1;

    for(;;)
    {
        try
        {
            // push
            m_LogQueue.push_back(LogEntry);
            break;
        }
        catch(...)
        {
            // destroy oldest entry if there is not enough memory
            P_EntryDeleteFirst();
        }
    }
}

void __stdcall CLog::P_EntryInsertVA(LOGENTRYTYPE nType, const char* lpszFmt, va_list lpVl)
{
    char szBuffer[8192];

    vsnprintf(szBuffer, __ARRAYSIZE(szBuffer), lpszFmt, lpVl);

    P_EntryInsert(nType, szBuffer);
}

void __stdcall CLog::P_EntryInsertBasicInfo(void)
{
    IncrementLevel();

    // Self
    const IMAGE_DOS_HEADER* lpIDH = (const IMAGE_DOS_HEADER*)GetModuleHandle(NULL);
    const IMAGE_NT_HEADERS* lpINH = (const IMAGE_NT_HEADERS*)(((const char*)lpIDH)+lpIDH->e_lfanew);

    LogInfo("Version: %lu.%lu.%lu.%lu (built: %08x, linker: %u.%u, subsystem: %hu.%hu, checksum: %08x)",
        APP_VERSIONINFO_VERSION,
        lpINH->FileHeader.TimeDateStamp,
        (UINT)lpINH->OptionalHeader.MajorLinkerVersion,
        (UINT)lpINH->OptionalHeader.MinorLinkerVersion,
        lpINH->OptionalHeader.MajorSubsystemVersion,
        lpINH->OptionalHeader.MinorSubsystemVersion,
        lpINH->OptionalHeader.CheckSum
    );

    // OS
    OSVERSIONINFO Osvi = { sizeof(Osvi) };
    GetVersionEx(&Osvi);

    char szProductName[128] = { 0 };
    DWORD dwBufferSize = __ARRAYSIZE(szProductName);
    HKEY hWindowsKey;

    if(RegOpenKeyExA(HKEY_LOCAL_MACHINE, Osvi.dwPlatformId!=VER_PLATFORM_WIN32_NT ? "Software\\Microsoft\\Windows\\CurrentVersion" : "Software\\Microsoft\\Windows NT\\CurrentVersion", 0, KEY_READ, &hWindowsKey)==ERROR_SUCCESS)
    {
        RegQueryValueExA(hWindowsKey, "ProductName", NULL, NULL, (unsigned char*)szProductName, &dwBufferSize);
        RegCloseKey(hWindowsKey);
    }

    if(!szProductName[0])
    {// generic name
        strcpy(szProductName, "Windows");
    }

    BOOL (WINAPI* IsWow64Process)(HANDLE,PBOOL) = (BOOL (WINAPI*)(HANDLE,PBOOL))GetProcAddress(GetModuleHandle("kernel32.dll"), "IsWow64Process");
    BOOL bIsWOW64 = FALSE;

    if(IsWow64Process)
    {
        IsWow64Process(GetModuleHandle(NULL), &bIsWOW64);
    }

    LogInfo("Operating System: %s %s (%lu.%lu.%lu, %s platform, %u-bit)",
        szProductName,
        Osvi.szCSDVersion,
        Osvi.dwMajorVersion,
        Osvi.dwMinorVersion,
        Osvi.dwPlatformId==VER_PLATFORM_WIN32_WINDOWS ? LOWORD(Osvi.dwBuildNumber) : Osvi.dwBuildNumber,
        CLog::P_Platform2Name(Osvi.dwPlatformId),
        bIsWOW64 ? 64 : 32
    );

    // Memory
    MEMORYSTATUS Ms = { sizeof(Ms) };
    GlobalMemoryStatus(&Ms);
    LogInfo("Memory Utilization: %lu%%", Ms.dwMemoryLoad);

#ifndef WITHOUT_INTEGRITY
    // Integrity Level
    int nIntegrityLevel;
    LogInfo("Integrity Level: %s", WNTGetProcessIntegrityLevel(GetCurrentProcess(), &nIntegrityLevel, NULL) ? CLog::P_Integrity2Name(nIntegrityLevel) : "Unsupported");
#endif

    // Misc
    LogInfo("Command Line: %s", GetCommandLineA());

    // Config
    g_Config.DumpToLog();

    DecrementLevel();
}

void __stdcall CLog::P_EntryDelete(LOGQUEUEIT It)
{
    LOGENTRY& LogEntry = *It;

    if(LogEntry.lpszMessage)
    {
        delete[] LogEntry.lpszMessage;
        LogEntry.lpszMessage = NULL;
    }

    m_LogQueue.erase(It);
}

void __stdcall CLog::P_EntryDeleteLast(void)
{
    P_EntryDelete(m_LogQueue.end()-1);
}

void __stdcall CLog::P_EntryDeleteFirst(void)
{
    P_EntryDelete(m_LogQueue.begin());
}

CLog::CLog(unsigned long luLimit)
    : m_luLimit(luLimit)
    , m_luOptions(0UL)
    , m_uLevel(1U)
{
    LogInfo("Logging started.");
    P_EntryInsertBasicInfo();
}

CLog::~CLog()
{
    LogInfo("Logging stopped.");

    if(m_luOptions&LOG_OPT_DEFERSAVE)
    {
        Store();
    }

    Clear();
}

void __cdecl CLog::LogMessage(const char* lpszFmt, ...)
{
    va_list lpVl;

    va_start(lpVl, lpszFmt);
    P_EntryInsertVA(LET_NONE, lpszFmt, lpVl);
    va_end(lpVl);
}

void __cdecl CLog::LogInfo(const char* lpszFmt, ...)
{
    va_list lpVl;

    va_start(lpVl, lpszFmt);
    P_EntryInsertVA(LET_INFO, lpszFmt, lpVl);
    va_end(lpVl);
}

void __cdecl CLog::LogWarning(const char* lpszFmt, ...)
{
    va_list lpVl;

    va_start(lpVl, lpszFmt);
    P_EntryInsertVA(LET_WARN, lpszFmt, lpVl);
    va_end(lpVl);
}

void __cdecl CLog::LogError(const char* lpszFmt, ...)
{
    va_list lpVl;

    va_start(lpVl, lpszFmt);
    P_EntryInsertVA(LET_FAIL, lpszFmt, lpVl);
    va_end(lpVl);
}

void __stdcall CLog::IncrementLevel(void)
{
    InterlockedIncrement((long*)&m_uLevel);
}

void __stdcall CLog::DecrementLevel(void)
{
    if(!InterlockedDecrement((long*)&m_uLevel))
    {
        DebugBreakHere();
    }
}

void __stdcall CLog::Store(bool bDefer)
{
    char* lpszDot;
    char szLogName[MAX_PATH];
    unsigned long luLen;

    if(bDefer)
    {
        m_luOptions|= LOG_OPT_DEFERSAVE;
        return;
    }

    luLen = GetModuleFileNameA(NULL, szLogName, __ARRAYSIZE(szLogName));

    if((lpszDot = strrchr(szLogName, '.'))==NULL)
    {
        lpszDot = &szLogName[luLen];
    }

    strcpy(lpszDot, ".log");

    HANDLE hMutex = CreateMutex(NULL, FALSE, "Global\\OpenSetupCLogMutex");

    if(hMutex!=NULL)
    {
        // request ownership
        WaitForSingleObject(hMutex, INFINITE);
    }

    FILE* hFile;

    if((hFile = fopen(szLogName, "a"))!=NULL)
    {
        fprintf(hFile, "== RO OpenSetup Log ============================================================\n");

        for(LOGQUEUEIT It = m_LogQueue.begin(); It!=m_LogQueue.end(); It++)
        {
            fprintf(hFile, "[%04hu-%02hu-%02hu %02hu:%02hu:%02hu.%03hu]%*s%s%s\n",
                // timestamp
                It->stTimeStamp.wYear, It->stTimeStamp.wMonth, It->stTimeStamp.wDay,
                It->stTimeStamp.wHour, It->stTimeStamp.wMinute, It->stTimeStamp.wSecond,
                It->stTimeStamp.wMilliseconds,
                // level
                It->uLevel*2, "",
                // type
                CLog::P_Type2Name(It->nType),
                // message
                It->lpszMessage);
        }

        fprintf(hFile, "================================================================================\n\n");
        fclose(hFile);
    }

    if(hMutex!=NULL)
    {
        // release ownership
        ReleaseMutex(hMutex);
        CloseHandle(hMutex);
        hMutex = NULL;
    }

    Clear();
}

void __stdcall CLog::Clear(void)
{
    while(m_LogQueue.size())
    {
        P_EntryDeleteLast();
    }
}
