// -----------------------------------------------------------------
// RagnarokOnline OpenSetup
// (c) 2010-2014 Ai4rei/AN
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
public:
    typedef enum LUAFILETYPE
    {
        LFT_AUTO = -1,  // try .lub, fallback to .lua
        LFT_LUB,        // try .lub only
        LFT_LUA,        // try .lua only
    }
    LUAFILETYPE;

protected:
    typedef struct LUAREADERINFO
    {
        const void* lpData;
        unsigned long luSize;
    }
    LUAREADERINFO, * LPLUAREADERINFO;

    lua_State* L;

public:
    typedef bool (__stdcall* LPFNFOREACHTABLEKEYFUNC)(CLuaIO* This, const char* lpszTable, const char* lpszKey, int nKeyType, int nValType, void* lpContext);

protected:
    void __stdcall P_GetField(int nIndex, const char* lpszName);
    void __stdcall P_GetTableField(const char* lpszTable, const char* lpszKey);

    static void* __stdcall Realloc(void* lpPtr, unsigned long luSize);
    static void __stdcall FreeEx(void** lpPtr);
    //
    static const char* __cdecl ReaderFunc(lua_State* L, void* lpData, size_t* lpuSize);
#ifndef DONT_USE_LUA_5_1
    static void* __cdecl AllocatorFunc(void* lpUserData, void* lpPtr, size_t uOldSize, size_t uNewSize);
#endif  /* DONT_USE_LUA_5_1 */
    void __stdcall Error(void);
    static bool __stdcall P_LoadBufferFromFile(const char* lpszFileName, unsigned long* lpluFileSize, void** lppBuf);

public:
    CLuaIO();
    ~CLuaIO();
    bool __stdcall Eval(const void* lpData, unsigned long luSize, const char* lpszName);
    bool __stdcall Eval(const char* lpszCode, const char* lpszName);
    bool __stdcall Load(const char* lpszName, LUAFILETYPE nFileType = LFT_AUTO);
    void __stdcall DefineTable(const char* lpszName);
    bool __stdcall GetInteger(const char* lpszName, unsigned long* lpluValue);
    bool __stdcall GetTableInteger(const char* lpszTable, const char* lpszName, unsigned long* lpluValue);
    bool __stdcall GetTableString(const char* lpszTable, const char* lpszName, char** lppszValue);
    void __stdcall SetTableInteger(const char* lpszTable, const char* lpszName, unsigned long luValue);
    void __stdcall DeleteTableValue(const char* lpszTable, const char* lpszName);
    void __stdcall ForEachTableKey(const char* lpszTable, LPFNFOREACHTABLEKEYFUNC Func, void* lpContext);
};

#endif  /* _LUAIO_H_ */
