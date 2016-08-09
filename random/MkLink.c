#include <stdio.h>
#include <windows.h>

typedef enum MKLINK_MODE
{
    MKLINK_MODE_HARDLINK,
    MKLINK_MODE_SYMLINK,
}
MKLINK_MODE;

static int __stdcall MkLink_P_HardLink(const char* lpszLnkName, const char* lpszDstName)
{
    DWORD dwAttr = GetFileAttributes(lpszDstName);

    if(dwAttr==INVALID_FILE_ATTRIBUTES)
    {
        printf("File does not exist or permission failed (code=%u).\n", GetLastError());
    }
    else
    if(dwAttr&FILE_ATTRIBUTE_DIRECTORY)
    {
        printf("Directories cannot be hard-linked, use junction instead.\n");
    }
    else
    if(CreateHardLink(lpszLnkName, lpszDstName, NULL))
    {
        printf("%s --> %s\n", lpszLnkName, lpszDstName);
        return EXIT_SUCCESS;
    }
    else
    {
        printf("Hard link could not be created (code=%u).\n", GetLastError());
    }

    return EXIT_FAILURE;
}

static int __stdcall MkLink_P_SymLink(const char* lpszLnkName, const char* lpszDstName)
{
    DWORD dwAttr = GetFileAttributes(lpszDstName);

    if(dwAttr==INVALID_FILE_ATTRIBUTES)
    {
        printf("File or directory does not exist or permission failed (code=%u).\n", GetLastError());
    }
    else
    if(CreateSymbolicLink(lpszLnkName, lpszDstName, (dwAttr&FILE_ATTRIBUTE_DIRECTORY) ? SYMBOLIC_LINK_FLAG_DIRECTORY : 0))
    {
        printf("%s --> %s\n", lpszLnkName, lpszDstName);
        return EXIT_SUCCESS;
    }
    else
    {
        printf("Symbolic link could not be created (code=%u).\n", GetLastError());
    }

    return EXIT_FAILURE;
}

static void __stdcall MkLink_P_ShowHelp(void)
{
    printf(
        "Creates a symbolic link or hard link to a file or directory.\n\n"
        "Usage: mklink [-h] <link name> <destination name>\n"
    );
}

int __cdecl main(int nArgc, char** lppszArgv)
{
    MKLINK_MODE Mode = MKLINK_MODE_HARDLINK;
    const char* lpszLnkName = NULL;
    const char* lpszDstName = NULL;
    int nIdx;

    for(nIdx = 1; nIdx<nArgc; nIdx++)
    {
        const char* lpszArg = lppszArgv[nIdx];

        if(lpszArg[0]!='-' && lpszArg[0]!='/')
        {
            if(!lpszLnkName)
            {
                lpszLnkName = lpszArg;
            }
            else
            if(!lpszDstName)
            {
                lpszDstName = lpszArg;
            }
            else
            {
                printf("Too many arguments.\n");
                return EXIT_FAILURE;
            }
        }
        else
        switch(lpszArg[1])
        {
            case '?':
                MkLink_P_ShowHelp();
                return EXIT_SUCCESS;
            case 'h':
            case 'H':
                Mode = MKLINK_MODE_HARDLINK;
                break;
            case 's':
            case 'S':
                printf("Symbolic links are not supported.\n");
                return EXIT_FAILURE;
            case 'j':
            case 'J':
                printf("Junctions are not supported.\n");
                return EXIT_FAILURE;
        }
    }

    if(!lpszLnkName || !lpszDstName)
    {
        printf("Insufficient arguments. Use 'mklink -?' for more information.\n");
        return EXIT_FAILURE;
    }

    switch(Mode)
    {
        //case MKLINK_MODE_SYMLINK: return MkLink_P_SymLink(lpszLnkName, lpszDstName);
        case MKLINK_MODE_HARDLINK: return MkLink_P_HardLink(lpszLnkName, lpszDstName);
    }

    printf("Unexpected runtime mode.\n");
    return EXIT_FAILURE;
}
