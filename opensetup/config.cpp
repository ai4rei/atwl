// -----------------------------------------------------------------
// RagnarokOnline OpenSetup
// (c) 2010-2013 Ai4rei/AN
// See doc/license.txt for details.
//
// -----------------------------------------------------------------

#include <string.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "opensetup.h"
#include "config.h"

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
