// -----------------------------------------------------------------
// RagnarokOnline OpenSetup
// (c) 2010-2013 Ai4rei/AN
// See doc/license.txt for details.
//
// -----------------------------------------------------------------

#ifndef _LUAIO_H_
#define _LUAIO_H_

extern "C"
{// since lua is C
    #include <lua.h>
}

// This specifies to use 5.0.2 library code, rather than archived
// 5.1.4, as compiled lubs are not compatible. To be removed once
// Gravity decides to update (if ever).
//#define DONT_USE_LUA_5_1

class CLuaIO
{
private:
    lua_State* L;
    //
    static void* __stdcall ReAlloc(void* lpPtr, unsigned long luSize);
    static void __stdcall FreeEx(void** lpPtr);
    //
    static const char* __cdecl ReaderFunc(lua_State* L, void* lpData, size_t* lpuSize);
#ifndef DONT_USE_LUA_5_1
    static void* __cdecl AllocatorFunc(void* lpUserData, void* lpPtr, size_t uOldSize, size_t uNewSize);
#endif  /* DONT_USE_LUA_5_1 */
    void __stdcall Error(void);
    static bool __stdcall FetchFile(const char* lpszName, void* lpBuf, unsigned long luSize);
    static bool __stdcall ExistFile(const char* lpszName, unsigned long* lpluSize);
public:
    CLuaIO();
    ~CLuaIO();
    bool __stdcall Load(const char* lpszName);
    void __stdcall DefineTable(const char* lpszName);
    bool __stdcall GetInteger(const char* lpszName, unsigned long* lpluValue);
    bool __stdcall GetTableInteger(const char* lpszTable, const char* lpszName, unsigned long* lpluValue);
};

#endif  /* _LUAIO_H_ */
