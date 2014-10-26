// -----------------------------------------------------------------
// RagnarokOnline OpenSetup
// (c) 2010-2014 Ai4rei/AN
// See doc/license.txt for details.
//
// -----------------------------------------------------------------

#include <stdio.h>
#include <string.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "opensetup.h"
#include "config.h"
#include "log.h"

// global instance
CConfig g_Config;

CConfig::CConfig()
{
    char* lpszDot;

    if(GetModuleFileNameA(NULL, this->m_szIniFile, __ARRAYSIZE(this->m_szIniFile)))
    {
        if((lpszDot = strrchr(this->m_szIniFile, '.'))==NULL)
        {
            lpszDot = this->m_szIniFile+strlen(this->m_szIniFile);
        }

        strcpy(lpszDot, ".ini");
    }
    else
    {// fallback
        strcpy(this->m_szIniFile, ".\\opensetup.ini");
    }
}

unsigned long __stdcall CConfig::GetString(const char* lpszSection, const char* lpszKey, const char* lpszDefault, char* lpszBuffer, unsigned long luBufferSize)
{
    return GetPrivateProfileStringA(lpszSection, lpszKey, lpszDefault, lpszBuffer, luBufferSize, this->m_szIniFile);
}

int __stdcall CConfig::GetNumber(const char* lpszSection, const char* lpszKey, int nDefault)
{
    return GetPrivateProfileIntA(lpszSection, lpszKey, nDefault, this->m_szIniFile);
}

bool __stdcall CConfig::GetBool(const char* lpszSection, const char* lpszKey, bool bDefault)
{
    return this->GetNumber(lpszSection, lpszKey, bDefault ? 1 : 0)!=0;
}

void __stdcall CConfig::DumpToLog(void)
{
    FILE* hFile = fopen(this->m_szIniFile, "r");

    if(hFile)
    {
        char szBuffer[2048];
        unsigned long luLine = 0;

        g_Log.LogInfo("Dumping contents of '%s':", this->m_szIniFile);
        g_Log.IncrementLevel();

        while(fgets(szBuffer, __ARRAYSIZE(szBuffer), hFile))
        {
            char* lpszEnd = szBuffer+strlen(szBuffer);

            // drop EOL
            while(lpszEnd!=szBuffer)
            {
                lpszEnd--;

                if(lpszEnd[0]=='\r' || lpszEnd[0]=='\n')
                {
                    lpszEnd[0] = 0;
                }
            }

            g_Log.LogMessage("%2u: %s", ++luLine, szBuffer);
        }

        g_Log.DecrementLevel();

        fclose(hFile);
    }
    else
    {
        g_Log.LogWarning("Either there is no config file or it failed to open.");
    }
}
