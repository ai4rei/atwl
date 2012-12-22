// -----------------------------------------------------------------
// NOTE: this is trimmed-down version with dependencies only
// Must not be used with other software than RO Open Setup.
// (c) 2009-2010 Ai4rei/AN
// -----------------------------------------------------------------

#ifndef _BTYPES_H_
#define _BTYPES_H_

/* bool for C */
#ifndef __cplusplus
    #ifndef __bool_true_false_are_defined
        typedef char bool;
        #define false ((bool)(1==0))
        #define true  ((bool)(1==1))
        #define __bool_true_false_are_defined
    #endif
#endif

/* vsnprintf */
#if defined(_MSC_VER) && _MSC_VER<1400
    #define strcmpi _strcmpi
#endif

/* modifiers substitutes */
#if defined(_MSC_VER)
    #define inline  __inline
    #define asm     __asm
#endif

/* because DebugBreak breaks in kernel */
inline void DebugBreakHere(void)
{
    asm
    {
        INT 3h
    }
    return;
}

#endif  /* _BTYPES_H_ */
