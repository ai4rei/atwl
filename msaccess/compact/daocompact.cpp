#include <windows.h>
#include <comutil.h>

// local copy of the DAO 3.6 DLL
#import "_dao360.dll" rename("BOF", "BoF") rename("EOF", "EoF")

static bool Dao360_CreateEngine(DAO::_DBEnginePtr& DBE)
{
    try
    {
        DBE = DAO::_DBEnginePtr(__uuidof(DAO::DBEngine));
    }
    catch(...)
    {
        return false;
    }

    return true;
}

static bool Dao360_CreateTemporaryWorkspace(const DAO::_DBEnginePtr& DBE, const _bstr_t& sUser, const _bstr_t& sPass, DAO::WorkspacePtr& WS)
{
    try
    {
        WS = DBE->CreateWorkspace("", sUser, sPass);
    }
    catch(...)
    {
        return false;
    }

    return true;
}

static bool Dao360_OpenDatabase(const DAO::WorkspacePtr& WS, const _bstr_t& sBase, DAO::DatabasePtr& DB)
{
    try
    {
        DB = WS->OpenDatabase(sBase);
    }
    catch(...)
    {
        return false;
    }

    return true;
}

static bool CompactDatabase(const _bstr_t& sSrcBase, const _bstr_t& sDstBase)
{
    DAO::_DBEnginePtr DBE;

    if(Dao360_CreateEngine(DBE))
    {
        try
        {
            DBE->CompactDatabase(sSrcBase, sDstBase);

            printf("Database compacted.\n");
        }
        catch(...)
        {
            printf("Error%s occured:\n", DBE->Errors->Count==1 ? "" :"s");

            if(DBE->Errors->Count)
            {
                for(long lIdx = 0; lIdx<DBE->Errors->Count; lIdx++)
                {
                    printf("- #%ld: %s (%ld, 0x%08lx)\n", lIdx, (const char*)(DBE->Errors->Item[lIdx]->Description), DBE->Errors->Item[lIdx]->Number, DBE->Errors->Item[lIdx]->Number);
                }
            }
            else
            {
                printf("- Unknown error.\n");
            }

            return false;
        }
    }
    else
    {
        printf("Failed to initialize DAO.\n");

        return false;
    }

    return true;
}

int __cdecl main(int nArgc, char** lppszArgv)
{
    bool bEoA = false;  // End of Args
    int nResult = EXIT_FAILURE;
    const char* lpszSrcBase = NULL;
    const char* lpszDstBase = NULL;

    for(int nIdx = 1; nIdx<nArgc; nIdx++)
    {
        const char* lpszArg = lppszArgv[nIdx];

        if(bEoA || (lpszArg[0]!='/' && lpszArg[0]!='-'))
        {
            if(!lpszSrcBase)
            {
                lpszSrcBase = lpszArg;
            }
            else
            if(!lpszDstBase)
            {
                lpszDstBase = lpszArg;
            }
            else
            {
                printf("Too many args.\n");
                return EXIT_FAILURE;
            }
        }
        else switch((++lpszArg)[0])
        {
            case '-':
                bEoA = true;
                break;
            default:
                printf("Unknown switch.\n");
                break;
        }
    }

    if(!lpszSrcBase || !lpszDstBase)
    {
        printf("Usage: daocompact <source database> <destination database>\n");
        return EXIT_FAILURE;
    }

    HRESULT hr = CoInitialize(NULL);

    if(!FAILED(hr))
    {
        if(CompactDatabase(lpszSrcBase, lpszDstBase))
        {
            nResult = EXIT_SUCCESS;
        }

        CoUninitialize();
    }
    else
    {
        printf("Failed to initialize COM.\n");
    }

    return nResult;
}
