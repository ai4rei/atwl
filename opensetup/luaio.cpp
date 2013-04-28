// -----------------------------------------------------------------
// RagnarokOnline OpenSetup
// (c) 2010-2013 Ai4rei/AN
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

typedef struct LUAREADERINFO_
{
    void* lpPtr;
    unsigned long luSize;
}
LUAREADERINFO, * LPLUAREADERINFO;

static const char* lpszAcceptableExtensions[] = { "lub", "lua" };

void* __stdcall CLuaIO::ReAlloc(void* lpPtr, unsigned long luSize)
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

    lpChunk = (const char*)lpReaderInfo->lpPtr;
    lpReaderInfo->lpPtr = ((char*)(lpReaderInfo->lpPtr))+luReqSize;
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

    return CLuaIO::ReAlloc(lpPtr, uNewSize);
}
#endif  /* DONT_USE_LUA_5_1 */

void __stdcall CLuaIO::Error(void)
{
    const char* lpszErrMsg;

    if(lua_gettop(this->L))
    {
        if(lua_isstring(this->L, -1) && (lpszErrMsg = lua_tostring(this->L, -1))!=NULL)
        {
            CError::ErrorMessageFromStringEx(NULL, TEXT_ERROR__TITLE, "%s", lpszErrMsg);
        }

        // pop the error message from stack (from lua_pcall)
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

bool __stdcall CLuaIO::FetchFile(const char* lpszName, void* lpBuf, unsigned long luSize)
{
    bool bSuccess = false;
    DWORD dwRead = 0;
    HANDLE hFile;

    if((hFile = CreateFile(lpszName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL))!=INVALID_HANDLE_VALUE)
    {
        if(ReadFile(hFile, lpBuf, luSize, &dwRead, NULL))
        {
            bSuccess = true;
        }

        CloseHandle(hFile);
    }

    return bSuccess;
}

bool __stdcall CLuaIO::ExistFile(const char* lpszName, unsigned long* lpluSize)
{
    bool bSuccess = false;
    HANDLE hFile;

    if((hFile = CreateFile(lpszName, 0, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL))!=INVALID_HANDLE_VALUE)
    {
        if((lpluSize[0] = GetFileSize(hFile, NULL))!=INVALID_FILE_SIZE)
        {
            bSuccess = true;
        }

        CloseHandle(hFile);
    }

    return bSuccess;
}

bool __stdcall CLuaIO::Load(const char* lpszName)
{
    char szScriptName[MAX_PATH];
    unsigned long luIdx, luFileSize = 0;
    void* lpBuf = NULL;

    if(this->L)
    {
        for(luIdx = 0; luIdx<__ARRAYSIZE(lpszAcceptableExtensions); luIdx++)
        {// look for a suitable file
            wsprintfA(szScriptName, "%s.%s", lpszName, lpszAcceptableExtensions[luIdx]);

            if(CLuaIO::ExistFile(szScriptName, &luFileSize))
            {
                if((lpBuf = CLuaIO::ReAlloc(NULL, luFileSize+1))!=NULL)
                {
                    if(CLuaIO::FetchFile(szScriptName, lpBuf, luFileSize))
                    {
                        ((char*)lpBuf)[luFileSize] = 0;
                        break;
                    }

                    CLuaIO::FreeEx(&lpBuf);
                }
            }
        }

        if(lpBuf)
        {
            int nResult;
            LUAREADERINFO ReaderInfo;

            ReaderInfo.lpPtr = lpBuf;
            ReaderInfo.luSize = luFileSize;

            nResult = lua_load(this->L, &CLuaIO::ReaderFunc, &ReaderInfo, lpszName);
            CLuaIO::FreeEx(&lpBuf);

            if(!nResult)
            {
                // load pushed the chunk as function on the stack
                if(!lua_pcall(this->L, 0, 0, 0))
                {
                    return true;
                }

                this->Error();
            }
        }
    }

    return false;
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
#ifndef DONT_USE_LUA_5_1
        lua_getfield(this->L, LUA_GLOBALSINDEX, lpszName);
#else  /* DONT_USE_LUA_5_1 */
        lua_pushstring(this->L, lpszName);
        lua_gettable(this->L, LUA_GLOBALSINDEX);
#endif  /* DONT_USE_LUA_5_1 */

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
#ifndef DONT_USE_LUA_5_1
        lua_getfield(this->L, LUA_GLOBALSINDEX, lpszTable);
        lua_getfield(this->L, -1, lpszName);  // push referenced index value
        lua_remove(this->L, -2);  // remove table from stack
#else  /* DONT_USE_LUA_5_1 */
        lua_pushstring(this->L, lpszTable);
        lua_gettable(this->L, LUA_GLOBALSINDEX);
        lua_pushstring(this->L, lpszName);
        lua_gettable(this->L, -1);
        lua_remove(this->L, -2);
#endif  /* DONT_USE_LUA_5_1 */

        if(lua_isnumber(this->L, -1))
        {
            lpluValue[0] = lua_tointeger(this->L, -1);
            bSuccess = true;
        }

        lua_pop(this->L, 1);
    }

    return bSuccess;
}
