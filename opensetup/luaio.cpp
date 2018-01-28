// -----------------------------------------------------------------
// RagnarokOnline OpenSetup
// (c) 2010+ Ai4rei/AN
// See doc/license.txt for details.
//
// -----------------------------------------------------------------

#include <stdlib.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "opensetup.h"
#include "error.h"
#include "luaio.h"
#include "resource.h"

static const char* l_lppszAcceptableExtensions[] = { "lub", "lua" };

void __stdcall CLuaIO::P_GetField(int nIndex, const char* lpszName)
{
#ifndef DONT_USE_LUA_5_1
    lua_getfield(this->L, nIndex, lpszName);
#else  /* DONT_USE_LUA_5_1 */
    lua_pushstring(this->L, lpszName);
    lua_gettable(this->L, nIndex);
#endif  /* DONT_USE_LUA_5_1 */
}

void __stdcall CLuaIO::P_GetTableField(const char* lpszTable, const char* lpszKey)
{
    P_GetField(LUA_GLOBALSINDEX, lpszTable);
    P_GetField(-1, lpszKey);

    lua_remove(this->L, -2);  // remove table from stack
}

void* __stdcall CLuaIO::Realloc(void* lpPtr, unsigned long luSize)
{
    return realloc(lpPtr, luSize);
}

void __stdcall CLuaIO::FreeEx(void** lpPtr)
{
    free(lpPtr[0]);
    lpPtr[0] = NULL;
}

const char* __cdecl CLuaIO::ReaderFunc(lua_State* L, void* lpData, size_t* lpuSize)
{
    const char* lpChunk;
    size_t luReqSize = lpuSize[0];
    LPLUAREADERINFO lpReaderInfo = (LPLUAREADERINFO)lpData;

    if(luReqSize>lpReaderInfo->luSize)
    {
        luReqSize = lpReaderInfo->luSize;
    }
    lpuSize[0] = luReqSize;

    lpChunk = (const char*)lpReaderInfo->lpData;
    lpReaderInfo->lpData = ((char*)(lpReaderInfo->lpData))+luReqSize;
    lpReaderInfo->luSize-= luReqSize;

    return lpChunk;
}

#ifndef DONT_USE_LUA_5_1
void* __cdecl CLuaIO::AllocatorFunc(void* lpUserData, void* lpPtr, size_t uOldSize, size_t uNewSize)
{
    if(lpPtr && uNewSize && uOldSize>=uNewSize)
    {// ref. "Lua assumes that the allocator never fails when uOldSize >= uNewSize."
        return lpPtr;
    }

    return CLuaIO::Realloc(lpPtr, uNewSize);
}
#endif  /* DONT_USE_LUA_5_1 */

void __stdcall CLuaIO::Error(void)
{
    const char* lpszErrMsg;

    if(lua_gettop(this->L))
    {
        if(lua_isstring(this->L, -1) && (lpszErrMsg = lua_tostring(this->L, -1))!=NULL)
        {
            CError::ErrorMessageFromStringEx(NULL, TEXT_ERROR__TITLE_LUA, "%s", lpszErrMsg);
        }

        // pop the error message from stack (from lua_load/lua_pcall)
        lua_pop(this->L, 1);
    }
}

CLuaIO::CLuaIO()
{
#ifndef DONT_USE_LUA_5_1
    this->L = lua_newstate(&CLuaIO::AllocatorFunc, NULL);
#else
    this->L = lua_open();
#endif  /* DONT_USE_LUA_5_1 */

    if(this->L)
    {
        ;
    }
    else
    {
        RaiseException(ERROR_OUTOFMEMORY, EXCEPTION_NONCONTINUABLE, 0, NULL);
    }
}

CLuaIO::~CLuaIO()
{
    if(this->L)
    {
        lua_close(this->L);
        this->L = NULL;
    }
}

bool __stdcall CLuaIO::P_LoadBufferFromFile(const char* lpszFileName, unsigned long* lpluFileSize, void** lppBuf)
{
    void* lpBuf = NULL;
    HANDLE hFile;

    if((hFile = CreateFileA(lpszFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL))!=INVALID_HANDLE_VALUE)
    {
        unsigned long luFileSize;

        if((luFileSize = GetFileSize(hFile, NULL))!=INVALID_FILE_SIZE)
        {
            // +1 for the unnecessary zero-termination
            if((lpBuf = CLuaIO::Realloc(NULL, luFileSize+1))!=NULL)
            {
                DWORD dwRead;

                if(ReadFile(hFile, lpBuf, luFileSize, &dwRead, NULL) && dwRead==luFileSize)
                {
                    ((char*)lpBuf)[luFileSize] = 0;  // zero-terminate

                    lpluFileSize[0] = luFileSize;
                    lppBuf[0]       = lpBuf;
                }
                else
                {
                    CLuaIO::FreeEx(&lpBuf);
                }
            }
        }

        CloseHandle(hFile);
    }

    return lpBuf!=NULL;
}

bool __stdcall CLuaIO::Eval(const void* lpData, unsigned long luSize, const char* lpszName)
{
    bool bSuccess = false;
    LUAREADERINFO Lri;

    Lri.lpData = lpData;
    Lri.luSize = luSize;

    if(lua_load(this->L, &CLuaIO::ReaderFunc, &Lri, lpszName))
    {
        this->Error();
    }
    else if(lua_pcall(this->L, 0, 0, 0))
    {
        this->Error();
    }
    else
    {
        bSuccess = true;
    }

    return bSuccess;
}

bool __stdcall CLuaIO::Eval(const char* lpszCode, const char* lpszName)
{
    return this->Eval((const void*)lpszCode, strlen(lpszCode), lpszName);
}

bool __stdcall CLuaIO::Load(const char* lpszName, LUAFILETYPE nFileType)
{
    bool bSuccess = false;
    char szFileName[MAX_PATH];
    unsigned long luIdx;
    unsigned long luFileSize;
    void* lpBuf = NULL;

    if(this->L)
    {
        switch(nFileType)
        {
            case LFT_AUTO:
                // look for a suitable file
                for(luIdx = 0; luIdx<__ARRAYSIZE(l_lppszAcceptableExtensions); luIdx++)
                {
                    wsprintfA(szFileName, "%s.%s", lpszName, l_lppszAcceptableExtensions[luIdx]);

                    if(CLuaIO::P_LoadBufferFromFile(szFileName, &luFileSize, &lpBuf))
                    {
                        break;
                    }
                }
                break;
            case LFT_LUB:
            case LFT_LUA:
                // specific type
                wsprintfA(szFileName, "%s.%s", lpszName, l_lppszAcceptableExtensions[nFileType]);
                CLuaIO::P_LoadBufferFromFile(szFileName, &luFileSize, &lpBuf);
                break;
        }

        if(lpBuf)
        {
            bSuccess = this->Eval(lpBuf, luFileSize, lpszName);

            CLuaIO::FreeEx(&lpBuf);
        }
    }

    return bSuccess;
}

void __stdcall CLuaIO::DefineTable(const char* lpszName)
{
    if(this->L)
    {
        lua_createtable(this->L, 0, 0);
        lua_setfield(this->L, LUA_GLOBALSINDEX, lpszName);
    }
}

bool __stdcall CLuaIO::GetInteger(const char* lpszName, unsigned long* lpluValue)
{
    bool bSuccess = false;

    if(this->L)
    {
        P_GetField(LUA_GLOBALSINDEX, lpszName);

        if(lua_isnumber(this->L, -1))
        {
            lpluValue[0] = lua_tointeger(this->L, -1);
            bSuccess = true;
        }

        lua_pop(this->L, 1);
    }

    return bSuccess;
}

bool __stdcall CLuaIO::GetTableInteger(const char* lpszTable, const char* lpszName, unsigned long* lpluValue)
{
    bool bSuccess = false;

    if(this->L)
    {
        P_GetTableField(lpszTable, lpszName);

        if(lua_isnumber(this->L, -1))
        {
            lpluValue[0] = lua_tointeger(this->L, -1);
            bSuccess = true;
        }

        lua_pop(this->L, 1);
    }

    return bSuccess;
}

bool __stdcall CLuaIO::GetTableString(const char* lpszTable, const char* lpszName, char** lppszValue)
{
    bool bSuccess = false;

    if(this->L)
    {
        P_GetTableField(lpszTable, lpszName);

        if(lua_isstring(this->L, -1))
        {
            lppszValue[0] = strdup(lua_tostring(this->L, -1));
            bSuccess = true;
        }

        lua_pop(this->L, 1);
    }

    return bSuccess;
}

void __stdcall CLuaIO::SetTableInteger(const char* lpszTable, const char* lpszName, unsigned long luValue)
{
    if(this->L)
    {
        P_GetField(LUA_GLOBALSINDEX, lpszTable);

#ifndef DONT_USE_LUA_5_1
        lua_pushinteger(this->L, luValue);
        lua_setfield(this->L, -2, lpszName);  // pops value from stack
#else  /* DONT_USE_LUA_5_1 */
        lua_pushstring(this->L, lpszName);
        lua_pushinteger(this->L, luValue);
        lua_settable(this->L, -3);  // pops value and key from stack
#endif  /* DONT_USE_LUA_5_1 */

        lua_pop(this->L, 1);  // remove table from stack
    }
}

void __stdcall CLuaIO::DeleteTableValue(const char* lpszTable, const char* lpszName)
{
    if(this->L)
    {
        P_GetField(LUA_GLOBALSINDEX, lpszTable);

#ifndef DONT_USE_LUA_5_1
        lua_pushnil(this->L);
        lua_setfield(this->L, -2, lpszName);  // pops value from stack
#else  /* DONT_USE_LUA_5_1 */
        lua_pushstring(this->L, lpszName);
        lua_pushnil(this->L);
        lua_settable(this->L, -3);  // pops value and key from stack
#endif  /* DONT_USE_LUA_5_1 */

        lua_pop(this->L, 1);  // remove table from stack
    }
}

void __stdcall CLuaIO::ForEachTableKey(const char* lpszTable, LPFNFOREACHTABLEKEYFUNC Func, void* lpContext)
{
    if(this->L)
    {
        P_GetField(LUA_GLOBALSINDEX, lpszTable);

        lua_pushnil(this->L);

        while(lua_next(this->L, -2)!=0)
        {
            bool bContinue = true;

            int nValType = lua_type(this->L, -1);

            lua_pop(this->L, 1);  // remove value from stack

            lua_pushvalue(this->L, -1);  // push a copy of the key

            int nKeyType = lua_type(this->L, -1);

            switch(nKeyType)
            {
                case LUA_TBOOLEAN:
                    bContinue = Func(this, lpszTable, lua_toboolean(this->L, -1) ? "true" : "false", nKeyType, nValType, lpContext);
                    break;
                case LUA_TNUMBER:
                case LUA_TSTRING:
                    bContinue = Func(this, lpszTable, lua_tostring(this->L, -1), nKeyType, nValType, lpContext);
                    break;
            }

            lua_pop(this->L, 1);  // remove key copy from stack
        }

        lua_pop(this->L, 2);  // remove key and table from stack
    }
}
