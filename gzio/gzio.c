#include <stdio.h>
#include <stdlib.h>

#include <btypes.h>
#include <bvgzio.h>

bool __WDECL freadPull(void* lpHandle, void* lpBuffer, size_t uBufferSize, size_t* lpuBufferPulled, void* lpContext)
{
    CONTEXTCAST(FILE*,hFile,lpHandle);
    CONTEXTCAST(size_t*,lpuLength,lpContext);
    size_t uRead;

    uRead = fread(lpBuffer, 1, uBufferSize, hFile);

    if(lpuBufferPulled)
    {
        lpuBufferPulled[0] = uRead;
    }
    else
    if(uBufferSize!=uRead)
    {
        return false;
    }

    lpuLength[0]+= uRead;

    return !ferror(hFile);
}

bool __WDECL fwritePush(void* lpHandle, const void* lpBuffer, size_t uBufferLength, void* lpContext)
{
    CONTEXTCAST(FILE*,hFile,lpHandle);
    CONTEXTCAST(size_t*,lpuLength,lpContext);
    size_t uWritten;

    uWritten = fwrite(lpBuffer, 1, uBufferLength, hFile);

    if(uBufferLength!=uWritten)
    {
        return false;
    }

    lpuLength[0]+= uWritten;

    return !ferror(hFile);
}

int main(int nArgc, char** lppszArgv)
{
    size_t uSrcLen = 0;
    size_t uDstLen = 0;
    FILE* hSrcFile;
    FILE* hDstFile;
    bool bSuccess = false;
    bool (__WDECL* DataAction)(const LPFNCOMMONI7EPULL3, void* const, void* const, const LPFNCOMMONI7EPUSH, void* const, void* const) = NULL;

    if(nArgc<3)
    {
        printf("Usage: gzio: <c(ompress) or d(ecompress)> <source file> <destination file>\n");

        return EXIT_FAILURE;
    }

    if(strcmp(lppszArgv[1], "c")==0)
    {
        DataAction = &BvGzIoDataSave;
    }
    else
    if(strcmp(lppszArgv[1], "d")==0)
    {
        DataAction = &BvGzIoDataLoad;
    }
    else
    {
        printf("Unknown action.\n");

        return EXIT_FAILURE;
    }

    if((hSrcFile = fopen(lppszArgv[2], "rb"))!=NULL)
    {
        if((hDstFile = fopen(lppszArgv[3], "wb"))!=NULL)
        {
            if(DataAction(&freadPull, hSrcFile, &uSrcLen, &fwritePush, hDstFile, &uDstLen))
            {
                bSuccess = true;
            }

            fclose(hDstFile);
        }

        fclose(hSrcFile);
    }

    if(bSuccess)
    {
        printf("Completed (%u -> %u).\n", uSrcLen, uDstLen);

        return EXIT_SUCCESS;
    }
    else
    {
        printf("Failed.\n");

        unlink(lppszArgv[3]);
    }

    return EXIT_FAILURE;
}
