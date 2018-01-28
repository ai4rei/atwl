// -----------------------------------------------------------------
// RagnarokOnline OpenSetup
// (c) 2010+ Ai4rei/AN
// See doc/license.txt for details.
//
// -----------------------------------------------------------------

#include <windows.h>

#include <versioninfo.h>

#include "opensetup.h"
#include "resource.h"
#include "sound.h"

bool __stdcall Sound::IsMSSSane(bool* lpbExist)
{
    static const char* z_lppszFileName[] =
    {
        "mss32.dll",    /* Miles Sound System */
        "mssfast.m3d",  /* Miles Fast 2D Positional Audio */
        "mp3dec.asi",   /* MP3 Decoder */
    };
    unsigned long luIdx;
    UINT uErrMode = SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOOPENFILEERRORBOX);

    /*
        Check for existence of all files.

        NOTE: *.asi, *.m3d and *.flt files are all loaded as found,
              not by any particular name. Due to the way Find*File()
              works, it loads all files that match *.asi*, *.m3d*
              and *.flt*
    */
    for(luIdx = 0; luIdx<__ARRAYSIZE(z_lppszFileName); luIdx++)
    {
        if(GetFileAttributes(z_lppszFileName[luIdx])==INVALID_FILE_ATTRIBUTES)
        {
            lpbExist[0] = false;

            SetErrorMode(uErrMode);
            return false;
        }
    }

    lpbExist[0] = true;

    /*
        Verify version info and "loadibility".
    */
    for(luIdx = 0; luIdx<__ARRAYSIZE(z_lppszFileName); luIdx++)
    {
        if(VersionInfoMatch(z_lppszFileName[luIdx], "ProductName", "Miles Sound System", true))
        {
            HMODULE hDll = LoadLibrary(z_lppszFileName[luIdx]);

            if(hDll)
            {
                FreeLibrary(hDll);

                /* success */
                continue;
            }
        }

        SetErrorMode(uErrMode);
        return false;
    }

    SetErrorMode(uErrMode);
    return true;
}
