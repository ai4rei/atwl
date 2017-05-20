#include <stdlib.h>

#include <btypes.h>

void* __WDECL MemAlloc(size_t uSize)
{
    return malloc(uSize);
}

void __WDECL MemFree(void* lpPtr)
{
    return free(lpPtr);
}
