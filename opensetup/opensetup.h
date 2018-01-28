// -----------------------------------------------------------------
// RagnarokOnline OpenSetup
// (c) 2010+ Ai4rei/AN
// See doc/license.txt for details.
//
// -----------------------------------------------------------------

#ifndef _OPENSETUP_H_
#define _OPENSETUP_H_

#ifndef vsnprintf
    #define vsnprintf _vsnprintf
#endif  /* vsnprintf */
#ifndef snprintf
    #define snprintf _snprintf
#endif  /* snprintf */

#define __ARRAYSIZE(x) (sizeof(x)/sizeof((x)[0]))

#define WIN32_VER_CHECK(lposvi,platform,major,minor) ( ( (lposvi)->dwPlatformId==(platform) ) && ( ( (lposvi)->dwMajorVersion>(major) ) || ( (lposvi)->dwMajorVersion==(major) && (lposvi)->dwMinorVersion>=(minor) ) ) )

extern HKEY HKEY_GRAVITY;

static inline void __stdcall DebugBreakHere(void)
{
    __asm int 3
    (void)DebugBreakHere;
}

static inline void __stdcall EnterSpinLock(LPLONG lplSL)
{
    while(InterlockedExchangeAdd(lplSL, 1))
    {
        InterlockedExchangeAdd(lplSL, -1);
        Sleep(1);
    }
    (void)EnterSpinLock;
}

static inline void __stdcall LeaveSpinLock(LPLONG lplSL)
{
    InterlockedExchangeAdd(lplSL, -1);
    Sleep(1);
    (void)LeaveSpinLock;
}

#endif  /* _OPENSETUP_H_ */
