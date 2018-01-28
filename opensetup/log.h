// -----------------------------------------------------------------
// RagnarokOnline OpenSetup
// (c) 2010+ Ai4rei/AN
// See doc/license.txt for details.
//
// -----------------------------------------------------------------

#ifndef _LOG_H_
#define _LOG_H_

#include <vector>

class CLog
{
protected:
    typedef enum LOGOPTION
    {
        LOG_OPT_DEFERSAVE = 0x1,
    }
    LOGOPTION;
    typedef enum LOGENTRYTYPE
    {
        LET_NONE,
        LET_INFO,
        LET_WARN,
        LET_FAIL,
    }
    LOGENTRYTYPE;
    typedef unsigned long LOGENTRYLEVEL;
    typedef struct LOGENTRY
    {
        char* lpszMessage;
        SYSTEMTIME stTimeStamp;
        LOGENTRYTYPE nType;
        LOGENTRYLEVEL uLevel;
    }
    LOGENTRY;
    typedef std::vector< LOGENTRY > LOGQUEUE;
    typedef LOGQUEUE::iterator LOGQUEUEIT;

    unsigned long m_luLimit;
    unsigned long m_luOptions;
    LOGENTRYLEVEL m_uLevel;
    LOGQUEUE m_LogQueue;

    static const char* __stdcall P_Platform2Name(DWORD dwPlatformId);
    static const char* __stdcall P_Integrity2Name(int nIntegrityLevel);
    static const char* __stdcall P_Type2Name(LOGENTRYTYPE nType);

    void __stdcall P_EntryInsert(LOGENTRYTYPE nType, const char* lpszMessage);
    void __stdcall P_EntryInsertVA(LOGENTRYTYPE nType, const char* lpszFmt, va_list lpVl);
    void __stdcall P_EntryInsertBasicInfo(void);
    void __stdcall P_EntryDelete(LOGQUEUEIT It);
    void __stdcall P_EntryDeleteLast(void);
    void __stdcall P_EntryDeleteFirst(void);

public:
    CLog(unsigned long luLimit = 0);
    ~CLog();
    //
    void __cdecl LogMessage(const char* lpszFmt, ...);
    void __cdecl LogInfo(const char* lpszFmt, ...);
    void __cdecl LogWarning(const char* lpszFmt, ...);
    void __cdecl LogError(const char* lpszFmt, ...);
    //
    void __stdcall IncrementLevel(void);
    void __stdcall DecrementLevel(void);
    //
    void __stdcall Store(bool bDefer = false);
    void __stdcall Clear(void);
};

extern CLog g_Log;

#endif  /* _LOG_H_ */
