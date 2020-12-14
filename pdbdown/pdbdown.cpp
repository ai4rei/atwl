#include <cstdio>

#include <windows.h>
#include <tchar.h>

// CodeView
#define CV_HEADER_NB10 '01BN'  // Visual C++ 6.0
#define CV_HEADER_RSDS 'SDSR'  // Visual C++ 7.0 (aka. .NET 2002)

typedef struct tagCV_HEADER
{
    DWORD dwHeader;                 // CV_HEADER_*
}
CV_HEADER, *PCV_HEADER;

typedef struct tagCV_NB10
{
    CV_HEADER Header;
    DWORD dwOffset;                 // 0 for NB10
    DWORD dwSignature;              // UNIX timestamp
    DWORD dwAge;                    // 1++
    CHAR pdb[1];                    // zero-terminated
}
CV_NB10, *PCV_NB10;

typedef struct tagCV_RSDS
{
    CV_HEADER Header;
    GUID Signature;                 // GUID
    DWORD dwAge;                    // 1++
    CHAR pdb[1];                    // zero-terminated
}
CV_RSDS, *PCV_RSDS;

enum FDI_ERROR
{
    FDI_ERROR_SUCCESS = 0,
    //
    FDI_ERROR_FAILURE,
    FDI_ERROR_CODEVIEW,
    FDI_ERROR_NO_CODEVIEW,
    FDI_ERROR_INVALID_RVA,
    FDI_ERROR_NO_DEBUG_DIRECTORY,
    FDI_ERROR_DATA_DIRECTORY,
    FDI_ERROR_OPTHDR_MAGIC_UNKNOWN,
    FDI_ERROR_SECTION_HEADER,
    FDI_ERROR_NO_SECTION,
    FDI_ERROR_DEBUG_DIRECTORY,
    FDI_ERROR_OPTHDR_MAGIC,
    FDI_ERROR_FILE_HEADER,
    FDI_ERROR_PE_SIGNATURE,
    FDI_ERROR_NT_HEADER,
    FDI_ERROR_OLD_DOS_HEADER,
    FDI_ERROR_DOS_HEADER,
    FDI_ERROR_MAPVIEWOFFILE,
    FDI_ERROR_CREATEFILEMAPPING,
    FDI_ERROR_GETFILESIZE,
    FDI_ERROR_CREATEFILE,
    //
    FDI_ERROR_UNKNOWN
};

LPCTSTR FdiErrorToString(FDI_ERROR const nError)
{
#define C2N(X) case X: return _T(#X)
    switch(nError)
    {
    C2N(FDI_ERROR_SUCCESS);
    //
    C2N(FDI_ERROR_FAILURE);
    C2N(FDI_ERROR_CODEVIEW);
    C2N(FDI_ERROR_NO_CODEVIEW);
    C2N(FDI_ERROR_INVALID_RVA);
    C2N(FDI_ERROR_NO_DEBUG_DIRECTORY);
    C2N(FDI_ERROR_DATA_DIRECTORY);
    C2N(FDI_ERROR_OPTHDR_MAGIC_UNKNOWN);
    C2N(FDI_ERROR_SECTION_HEADER);
    C2N(FDI_ERROR_NO_SECTION);
    C2N(FDI_ERROR_DEBUG_DIRECTORY);
    C2N(FDI_ERROR_OPTHDR_MAGIC);
    C2N(FDI_ERROR_FILE_HEADER);
    C2N(FDI_ERROR_PE_SIGNATURE);
    C2N(FDI_ERROR_NT_HEADER);
    C2N(FDI_ERROR_OLD_DOS_HEADER);
    C2N(FDI_ERROR_DOS_HEADER);
    C2N(FDI_ERROR_MAPVIEWOFFILE);
    C2N(FDI_ERROR_CREATEFILEMAPPING);
    C2N(FDI_ERROR_GETFILESIZE);
    C2N(FDI_ERROR_CREATEFILE);
    //
    C2N(FDI_ERROR_UNKNOWN);
    }
#undef C2N

    return _T("<<unknown>>");
}

struct FILEDEBUGINFO
{
    HANDLE hFileMap;
    HANDLE hFile;
    PIMAGE_SECTION_HEADER pISH;
    PCV_HEADER pCV;
    LPVOID lpImageBase;
    DWORD dwISHCount;
    DWORD dwImageLength;
    DWORD dwCVLength;
};

#define IMAGE_CONTAINS_FIELD(StructPointer, StructMember, ImageBase, ImageLength) ( ( (ULONG_PTR)(StructPointer) >= (ULONG_PTR)(ImageBase) ) && ( (ULONG_PTR)(&(StructPointer)->StructMember) + sizeof( (StructPointer)->StructMember ) ) <= ( (ULONG_PTR)(ImageBase) + (ImageLength) ) )
#define IMAGE_CONTAINS_LENGTH(StructPointer, StructLength, ImageBase, ImageLength) ( ( (ULONG_PTR)(StructPointer) >= (ULONG_PTR)(ImageBase) ) && ( (ULONG_PTR)(StructPointer) + (StructLength) ) <= ( (ULONG_PTR)(ImageBase) + (ImageLength) ) )

LPVOID GetVAFromRVA(LPVOID const lpImageBase, DWORD const dwImageLength, DWORD const dwRVA, DWORD const dwRVASize, FILEDEBUGINFO* const lpFdi, PIMAGE_SECTION_HEADER* const lppISHOut = NULL)
{
    for( DWORD dwIdx = 0; dwIdx<lpFdi->dwISHCount; dwIdx++ )
    {
        PIMAGE_SECTION_HEADER const pISH = &lpFdi->pISH[dwIdx];

        if( pISH->Characteristics & ( IMAGE_SCN_CNT_INITIALIZED_DATA | IMAGE_SCN_CNT_CODE ) )
        {
            if( IMAGE_CONTAINS_LENGTH(dwRVA, dwRVASize, pISH->VirtualAddress, pISH->Misc.VirtualSize) )
            {
                if( lppISHOut )
                {
                    lppISHOut[0] = pISH;
                }

                return (LPVOID)((ULONG_PTR)(lpImageBase) + pISH->PointerToRawData + ( dwRVA - pISH->VirtualAddress ));
            }
        }
    }

    return NULL;
}

FDI_ERROR GetDataDirectoryDebugInfo(LPVOID const lpImageBase, DWORD const dwImageLength, PIMAGE_DATA_DIRECTORY const pIDED, FILEDEBUGINFO* const lpFdi)
{
    FDI_ERROR nError = FDI_ERROR_UNKNOWN;
    LPVOID const lpAddress = GetVAFromRVA(lpImageBase, dwImageLength, pIDED->VirtualAddress, pIDED->Size, lpFdi);

    if( lpAddress != NULL && IMAGE_CONTAINS_LENGTH(lpAddress, pIDED->Size, lpImageBase, dwImageLength) )
    {
        PIMAGE_DEBUG_DIRECTORY const pDD = (PIMAGE_DEBUG_DIRECTORY)lpAddress;
        DWORD const dwDDCount = pIDED->Size/sizeof(pDD[0]);

        for( DWORD dwIdx = 0; dwIdx<dwDDCount; dwIdx++ )
        {
            PIMAGE_DEBUG_DIRECTORY const pDDE = &pDD[dwIdx];

            if( pDDE->Type == IMAGE_DEBUG_TYPE_CODEVIEW )
            {
                PCV_HEADER pCV = (PCV_HEADER)((ULONG_PTR)(lpImageBase) + pDDE->PointerToRawData);

                if( IMAGE_CONTAINS_LENGTH(pCV, pDDE->SizeOfData, lpImageBase, dwImageLength) )
                {
                    lpFdi->dwCVLength = pDDE->SizeOfData;
                    lpFdi->pCV = pCV;

                    nError = FDI_ERROR_SUCCESS;
                }
                else
                {
                    nError = FDI_ERROR_CODEVIEW;
                }

                break;
            }
        }

        if( dwIdx == dwDDCount )
        {
            nError = FDI_ERROR_NO_CODEVIEW;
        }
    }
    else
    {
        nError = FDI_ERROR_INVALID_RVA;
    }

    return nError;
}

template< typename T > PIMAGE_DATA_DIRECTORY GetImageIDD(LPVOID const lpImageBase, DWORD const dwImageLength, PIMAGE_FILE_HEADER const pIFH, T* const pINH, DWORD* const lpdwIDDCount)
{
    if( IMAGE_CONTAINS_LENGTH(pINH, pIFH->SizeOfOptionalHeader, lpImageBase, dwImageLength) )
    {
        if( pINH->OptionalHeader.NumberOfRvaAndSizes )
        {
            if( IMAGE_CONTAINS_FIELD(&pINH->OptionalHeader, DataDirectory[pINH->OptionalHeader.NumberOfRvaAndSizes-1U], &pINH->OptionalHeader, pIFH->SizeOfOptionalHeader) )
            {
                lpdwIDDCount[0] = pINH->OptionalHeader.NumberOfRvaAndSizes;

                return &pINH->OptionalHeader.DataDirectory[0];
            }
        }
    }

    return NULL;
}

FDI_ERROR GetImageDebugInfo(LPVOID const lpImageBase, DWORD const dwImageLength, FILEDEBUGINFO* const lpFdi)
{
    FDI_ERROR nError = FDI_ERROR_UNKNOWN;
    PIMAGE_DOS_HEADER const pIDH = (PIMAGE_DOS_HEADER)(lpImageBase);

    if( IMAGE_CONTAINS_FIELD(pIDH, e_lfanew, lpImageBase, dwImageLength) && pIDH->e_magic == IMAGE_DOS_SIGNATURE )
    {
        if( pIDH->e_lfarlc >= sizeof(pIDH) )
        {
            PIMAGE_NT_HEADERS const pINH = (PIMAGE_NT_HEADERS)((ULONG_PTR)(pIDH) + pIDH->e_lfanew);

            if( IMAGE_CONTAINS_FIELD(pINH, Signature, lpImageBase, dwImageLength) )
            {
                if( pINH->Signature == IMAGE_NT_SIGNATURE )
                {// PE32/PE32+
                    if( IMAGE_CONTAINS_FIELD(pINH, FileHeader, lpImageBase, dwImageLength) )
                    {
                        // if( pINH->FileHeader.Characteristics & IMAGE_FILE_DEBUG_STRIPPED )
                        {
                            if( IMAGE_CONTAINS_FIELD(pINH, OptionalHeader.Magic, lpImageBase, dwImageLength) )
                            {
                                DWORD dwIDDCount = 0;
                                PIMAGE_DATA_DIRECTORY pIDD = NULL;

                                switch(pINH->OptionalHeader.Magic)
                                {
                                case IMAGE_NT_OPTIONAL_HDR32_MAGIC:  // 0x10-Bits (0x10B) - PE32
                                    pIDD = GetImageIDD(lpImageBase, dwImageLength, &pINH->FileHeader, (PIMAGE_NT_HEADERS32)pINH, &dwIDDCount);
                                    break;
                                case IMAGE_NT_OPTIONAL_HDR64_MAGIC:  // 0x20-Bits (0x20B) - PE32+
                                    pIDD = GetImageIDD(lpImageBase, dwImageLength, &pINH->FileHeader, (PIMAGE_NT_HEADERS64)pINH, &dwIDDCount);
                                    break;
                                default:
                                    nError = FDI_ERROR_OPTHDR_MAGIC_UNKNOWN;
                                    break;
                                }

                                if( pIDD != NULL )
                                {
                                    if( dwIDDCount > IMAGE_DIRECTORY_ENTRY_DEBUG )
                                    {
                                        PIMAGE_DATA_DIRECTORY const pIDED = &pIDD[IMAGE_DIRECTORY_ENTRY_DEBUG];

                                        if( pIDED->Size != 0 )
                                        {
                                            if( pINH->FileHeader.NumberOfSections )
                                            {
                                                PIMAGE_SECTION_HEADER pISH = IMAGE_FIRST_SECTION(pINH);

                                                if( IMAGE_CONTAINS_FIELD(&pISH[pINH->FileHeader.NumberOfSections-1U], Characteristics, lpImageBase, dwImageLength) )
                                                {
                                                    lpFdi->dwISHCount = pINH->FileHeader.NumberOfSections;
                                                    lpFdi->pISH = pISH;

                                                    nError = GetDataDirectoryDebugInfo(lpImageBase, dwImageLength, pIDED, lpFdi);
                                                }
                                                else
                                                {
                                                    nError = FDI_ERROR_SECTION_HEADER;
                                                }
                                            }
                                            else
                                            {
                                                nError = FDI_ERROR_NO_SECTION;
                                            }
                                        }
                                        else
                                        {
                                            nError = FDI_ERROR_NO_DEBUG_DIRECTORY;
                                        }
                                    }
                                    else
                                    {
                                        nError = FDI_ERROR_DEBUG_DIRECTORY;
                                    }
                                }
                                else
                                {
                                    nError = FDI_ERROR_DATA_DIRECTORY;
                                }
                            }
                            else
                            {
                                nError = FDI_ERROR_OPTHDR_MAGIC;
                            }
                        }
                    }
                    else
                    {
                        nError = FDI_ERROR_FILE_HEADER;
                    }
                }
                else
                {// NE or ROM
                    nError = FDI_ERROR_PE_SIGNATURE;
                }
            }
            else
            {
                nError = FDI_ERROR_NT_HEADER;
            }
        }
        else
        {
            nError = FDI_ERROR_OLD_DOS_HEADER;
        }
    }
    else
    {
        nError = FDI_ERROR_DOS_HEADER;
    }

    return nError;
}

FDI_ERROR GetFileDebugInfo(LPCTSTR const lpszFileName, FILEDEBUGINFO* const lpFdi)
{
    FDI_ERROR nError = FDI_ERROR_UNKNOWN;

    lpFdi->hFile = CreateFile(lpszFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_RANDOM_ACCESS, NULL);

    if( lpFdi->hFile != INVALID_HANDLE_VALUE )
    {
        lpFdi->dwImageLength = GetFileSize(lpFdi->hFile, NULL);

        if( lpFdi->dwImageLength != INVALID_FILE_SIZE )
        {
            lpFdi->hFileMap = CreateFileMapping(lpFdi->hFile, NULL, PAGE_READONLY, 0, 0, NULL);

            if( lpFdi->hFileMap != NULL )
            {
                lpFdi->lpImageBase = MapViewOfFile(lpFdi->hFileMap, FILE_MAP_READ, 0, 0, 0);

                if( lpFdi->lpImageBase != NULL )
                {
                    nError = GetImageDebugInfo(lpFdi->lpImageBase, lpFdi->dwImageLength, lpFdi);

                    if( nError != FDI_ERROR_SUCCESS )
                    {
                        UnmapViewOfFile(lpFdi->lpImageBase);
                        lpFdi->lpImageBase = NULL;
                    }
                }
                else
                {
                    nError = FDI_ERROR_MAPVIEWOFFILE;
                }

                if( nError != FDI_ERROR_SUCCESS )
                {
                    CloseHandle(lpFdi->hFileMap);
                    lpFdi->hFileMap = NULL;
                }
            }
            else
            {
                nError = FDI_ERROR_CREATEFILEMAPPING;
            }
        }
        else
        {
            nError = FDI_ERROR_GETFILESIZE;
        }

        if( nError != FDI_ERROR_SUCCESS )
        {
            CloseHandle(lpFdi->hFile);
            lpFdi->hFile = INVALID_HANDLE_VALUE;
        }
    }
    else
    {
        nError = FDI_ERROR_CREATEFILE;
    }

    return nError;
}

VOID FreeFileDebugInfo(FILEDEBUGINFO* const lpFdi)
{
    if( lpFdi->lpImageBase != NULL )
    {
        UnmapViewOfFile(lpFdi->lpImageBase);
        lpFdi->lpImageBase = NULL;
    }
    if( lpFdi->hFileMap != NULL )
    {
        CloseHandle(lpFdi->hFileMap);
        lpFdi->hFileMap = NULL;
    }
    if( lpFdi->hFile != INVALID_HANDLE_VALUE )
    {
        CloseHandle(lpFdi->hFile);
        lpFdi->hFile = INVALID_HANDLE_VALUE;
    }
}

void __cdecl _tmain(int nArgc, TCHAR** lppszArgv)
{
    for( int nIdx = 1; nIdx<nArgc; nIdx++ )
    {
        FDI_ERROR nError;
        FILEDEBUGINFO Fdi = { 0 };
        TCHAR const* const lpszArg = lppszArgv[nIdx];

        _tprintf(_T("%s: "), lpszArg);

        if( ( nError = GetFileDebugInfo(lpszArg, &Fdi) ) == FDI_ERROR_SUCCESS )
        {
            _tprintf(_T("'%.4s'"), &Fdi.pCV->dwHeader);
            FreeFileDebugInfo(&Fdi);
        }
        else
        {
            _tprintf(_T("%s"), FdiErrorToString(nError));
        }

        _putts(_T(""));
    }
}
