/* -----------------------------------------------------------------
// CRagEffect::Init Database
// (c) 2014 Ai4rei/AN
//
// ---------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define DBG
#include <btypes.h>
#include <bvdebug.h>
#include <bvpe.h>
#include <cstr.h>
#include <hashdb.h>
#include <memory.h>
#include <ragtok.h>
#include <ioapix.h>

#define SIGNATURE "gravity"

static char** l_lppszCells = NULL;
static SIZE_T l_uRows = 0;
static struct hashdb* l_EffectDB = NULL;
static SIZE_T l_uEndOffset = 0;

static void __WDECL MeConstructName(char* lpszBuffer, int nIsMinEffect, SIZE_T uIdx)
{
    const char* lpszName = (!nIsMinEffect || !(l_lppszCells[uIdx*3+2])[0]) ? l_lppszCells[uIdx*3+1] : l_lppszCells[uIdx*3+2];
    const char* lpszRand;

    if((lpszRand = strchr(lpszName, ':'))!=NULL)
    {/* formatted and random */
        int nMin = 0, nMax = 0;

        if(sscanf(lpszRand+1, "%d,%d", &nMin, &nMax)==2)
        {
            int nRand = nMin+rand()%(nMax-nMin+1);
            
            sprintf(lpszBuffer, lpszName, nRand);
            strcpy(strchr(lpszBuffer, ':'), ".str");
        }
        else
        {
            DBGPRINT(("RagEffect::MeConstructName: Invalid random format, fallback to unformatted.\n"));
            sprintf(lpszBuffer, "%.*s.str", lpszRand-lpszName, lpszName);
        }
    }
    else
    {
        /* normal */
        sprintf(lpszBuffer, "%s.str", lpszName);
    }
}

static void __CDECL DllCallback(char* lpszBuffer, int nIsMinEffect, int nSkillEffect)
{
    SIZE_T uIdx;

    if(l_uEndOffset==81)
    {/* make up for shifted stack reference after hot-patch (ESP referenced stack vars) */
        lpszBuffer+= 4;
    }

    if(l_EffectDB==NULL)
    {/* initialize cache */
        DBGPRINT(("RagEffect::DllCallback: Initializing cache...\n"));

        if((l_EffectDB = idb_alloc(51, 0, 0))==NULL)
        {
            DBGPRINT(("RagEffect::DllCallback: - Cache out of memory, performance will be degraded.\n"));
        }
        else for(uIdx = 0; uIdx<l_uRows; uIdx++)
        {
            const char* lpszItem = l_lppszCells[uIdx*3+0];
            const char* lpszNext;
            long lID;

            lID = lstrtolA(lpszItem, &lpszNext, 10);
            
            if(!lID && lpszItem==lpszNext)
            {
                DBGPRINT(("RagEffect::DllCallback: - Invalid ID '%s', skipping.\n", lpszItem));
                continue;
            }
            
            if(!idb_put(l_EffectDB, lID, (void*)uIdx))
            {
                DBGPRINT(("RagEffect::DllCallback: - Cache out of memory, remaining entries will not be cached.\n"));
                break;
            }

            DBGPRINT(("RagEffect::DllCallback: - ID[%d] = '%s.str'/'%s.str'.\n", lID, l_lppszCells[uIdx*3+1], l_lppszCells[uIdx*3+2]));
        }

        DBGPRINT(("RagEffect::DllCallback: Cache initialization complete.\n"));
    }

    DBGPRINT(("RagEffect::DllCallback: Request for ID[%d].\n", nSkillEffect));

    if(l_EffectDB)
    {
        if(idb_exists(l_EffectDB, nSkillEffect, (void**)&uIdx))
        {
            MeConstructName(lpszBuffer, nIsMinEffect, uIdx);
            DBGPRINT(("RagEffect::DllCallback: Cache HIT! '%s'.\n", lpszBuffer));
            return;
        }
    }

    for(uIdx = 0; uIdx<l_uRows; uIdx++)
    {
        const char* lpszItem = l_lppszCells[uIdx*3+0];
        const char* lpszNext;
        long lID;

        lID = lstrtolA(lpszItem, &lpszNext, 10);

        if(!lID && lpszItem==lpszNext)
        {
            DBGPRINT(("RagEffect::DllCallback: - Invalid ID '%s', skipping.\n", lpszItem));
            continue;
        }

        if(nSkillEffect==lID)
        {
            MeConstructName(lpszBuffer, nIsMinEffect, uIdx);
            DBGPRINT(("RagEffect::DllCallback: Table HIT! '%s'.\n", lpszBuffer));
            return;
        }
    }

    DBGPRINT(("RagEffect::DllCallback: ID[%d] not found.\n", nSkillEffect));
}

static void __WDECL DllInstall(void)
{
    BVPEINFO PE;

    DBGPRINT(("RagEffect::DllInstall: Installing table support...\n"));

    srand(GetTickCount());

    if(BvPEGet(GetModuleHandle(NULL), &PE))
    {
        DBGPRINT(("RagEffect::DllInstall: Code at %#x in %u (%#x) bytes, Data at %#x in %u (%#x) bytes.\n", PE.lpCodeBase, PE.uCodeSize, PE.uCodeSize, PE.lpDataBase, PE.uDataSize, PE.uDataSize));

        /*
            "hello, are you sane?"
        */
        if(Memory_Match(PE.lpDataBase, PE.uDataSize, SIGNATURE, sizeof(SIGNATURE)))
        {
            const BYTE ucMatch[] =
            {
                0x83,0xC4,0x08,                      /* ADD     ESP,8h  */
                0x83,0xF8,0x40,                      /* CMP     EAX,40h */
                0x0F,0x8F,0x00,0x00,0x00,0x00,       /* JG      v       */
                0x0F,0x84,0x00,0x00,0x00,0x00,       /* JE      v       */
                0x83,0xE8,0x0A,                      /* SUB     EAX,0Ah */
                0x83,0xF8,0x2A,                      /* CMP     EAX,2Ah */
                0x0F,0x87,                           /* JA      v       */
            };
            const BYTE ucWildM[] =
            {
                0xFF,0xFF,0xFF,
                0xFF,0xFF,0xFF,
                0xFF,0xFF,0x00,0x00,0x00,0x00,
                0xFF,0xFF,0x00,0x00,0x00,0x00,
                0xFF,0xFF,0xFF,
                0xFF,0xFF,0xFF,
                0xFF,0xFF,
            };
            LPVOID lpSwitchHead = (LPVOID)Memory_MatchWild(PE.lpCodeBase, PE.uCodeSize, ucMatch, sizeof(ucMatch), ucWildM);

            if(lpSwitchHead)
            {

                if(((LPBYTE)MAKEOFFSET(lpSwitchHead, 81))[0]==0xE9)
                {/* VC9 */
                    l_uEndOffset = 81;
                }
                else
                if(((LPBYTE)MAKEOFFSET(lpSwitchHead, 80))[0]==0xE9)
                {/* VC10 */
                    l_uEndOffset = 80;
                }

                if(l_uEndOffset)
                {
                    unsigned long luSize;

                    /*
                        load and parse data
                    */
                    if(IOFileSizeExists("data\\id2rageffect.txt", &luSize))
                    {
                        char* lpszTable = Memory_Alloc(luSize+1);

                        if(lpszTable)
                        {
                            if(IOFileGetContents("data\\id2rageffect.txt", lpszTable, luSize))
                            {
                                RAGTOK_ERROR Error;

                                lpszTable[luSize] = 0;

                                Error = RagTokenParse(lpszTable, 3, &l_lppszCells, &l_uRows);

                                if(IS_S_CODE(Error))
                                {
                                    /*
                                        install callback
                                    */
                                    DWORD dwOldProtect;

                                    if(VirtualProtect(lpSwitchHead, l_uEndOffset, PAGE_READWRITE, &dwOldProtect))
                                    {
                                        /* jump over switch head */
                                        ((LPBYTE)MAKEOFFSET(lpSwitchHead,3))[0] = 0xEB;   /* CMP -> */
                                        ((LPBYTE)MAKEOFFSET(lpSwitchHead,4))[0] = 0x36;   /*    JMP */
                                        ((LPBYTE)MAKEOFFSET(lpSwitchHead,5))[0] = 0x90;   /* cosmetic patch */

                                        /* pass skill effect id */
                                        ((LPBYTE)MAKEOFFSET(lpSwitchHead,58))[0] = 0x90;  /* cosmetic patch */
                                        ((LPBYTE)MAKEOFFSET(lpSwitchHead,59))[0] = 0x50;  /* PUSH EAX */

                                        /* pass mineffect value */
                                        Memory_Copy(MAKEOFFSET(lpSwitchHead,60), MAKEOFFSET(lpSwitchHead,l_uEndOffset+5), 2+4);
                                        ((LPBYTE)MAKEOFFSET(lpSwitchHead,60))[0] = 0xFF;  /* CMP -> */
                                        ((LPBYTE)MAKEOFFSET(lpSwitchHead,61))[0] = 0x35;  /*   PUSH */

                                        /* inject callback into call */
                                        ((LPBYTE)MAKEOFFSET(lpSwitchHead,l_uEndOffset-11))[0] = 0xBE;  /* PUSH -> MOV */
                                        ((LPDWORD)MAKEOFFSET(lpSwitchHead,l_uEndOffset-10))[0] = (DWORD)&DllCallback;

                                        VirtualProtect(lpSwitchHead, l_uEndOffset, dwOldProtect, &dwOldProtect);

                                        DBGPRINT(("RagEffect::DllInstall: Table support installed (Head=%#x, Tail=%u).\n", lpSwitchHead, l_uEndOffset));
                                    }
                                    else
                                    {
                                        MessageBox(NULL, "Fatal Error: Cannot unprotect client code.", "RagEffect::DllInstall", MB_OK|MB_ICONINFORMATION);
                                    }
                                }
                                else
                                {
                                    MessageBox(NULL, "Error: Cannot parse table.", "RagEffect::DllInstall", MB_OK|MB_ICONINFORMATION);
                                }
                            }
                            else
                            {
                                MessageBox(NULL, "Error: Cannot read table.", "RagEffect::DllInstall", MB_OK|MB_ICONINFORMATION);
                            }

                            Memory_FreeEx(&lpszTable);
                        }
                        else
                        {
                            MessageBox(NULL, "Fatal Error: Out of memory.", "RagEffect::DllInstall", MB_OK|MB_ICONINFORMATION);
                        }
                    }
                    else
                    {
                        MessageBox(NULL, "Error: Table not found.", "RagEffect::DllInstall", MB_OK|MB_ICONINFORMATION);
                    }
                }
                else
                {
                    MessageBox(NULL, "Error: No tail for hot-patch.", "RagEffect::DllInstall", MB_OK|MB_ICONINFORMATION);
                }
            }
            else
            {
                MessageBox(NULL, "Error: No match for hot-patch.", "RagEffect::DllInstall", MB_OK|MB_ICONINFORMATION);
            }
        }
        else
        {
            MessageBox(NULL, "Error: Client is not sane.", "RagEffect::DllInstall", MB_OK|MB_ICONINFORMATION);
        }
    }
    else
    {
        MessageBox(NULL, "Error: Code or data section not found.", "RagEffect::DllInstall", MB_OK|MB_ICONINFORMATION);
    }
}

BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
    switch(dwReason)
    {
        case DLL_PROCESS_ATTACH:
            DisableThreadLibraryCalls(hInstance);

            DllInstall();
            break;
        case DLL_PROCESS_DETACH:
            if(l_EffectDB)
            {
                hashdb_destroy(l_EffectDB);
            }
            if(l_lppszCells)
            {
                Memory_FreeEx(&l_lppszCells);
            }
            break;
    }

    return TRUE;
}
