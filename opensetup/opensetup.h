// -----------------------------------------------------------------
// RagnarokOnline OpenSetup
// (c) 2010-2013 Ai4rei/AN
// See doc/license.txt for details.
//
// -----------------------------------------------------------------

#ifndef _OPENSETUP_H_
#define _OPENSETUP_H_

#define __ARRAYSIZE(x) (sizeof(x)/sizeof((x)[0]))

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
        Sleep(100);
    }
    (void)EnterSpinLock;
}

static inline void __stdcall LeaveSpinLock(LPLONG lplSL)
{
    InterlockedExchangeAdd(lplSL, -1);
    Sleep(100);
    (void)LeaveSpinLock;
}

#endif  /* _OPENSETUP_H_ */
