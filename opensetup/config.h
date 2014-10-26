// -----------------------------------------------------------------
// RagnarokOnline OpenSetup
// (c) 2010-2014 Ai4rei/AN
// See doc/license.txt for details.
//
// -----------------------------------------------------------------

#ifndef _CONFIG_H_
#define _CONFIG_H_

class CConfig
{
private:
    char m_szIniFile[MAX_PATH];

public:
    CConfig();
    //
    unsigned long __stdcall GetString(const char* lpszSection, const char* lpszKey, const char* lpszDefault, char* lpszBuffer, unsigned long luBufferSize);
    int __stdcall GetNumber(const char* lpszSection, const char* lpszKey, int nDefault);
    bool __stdcall GetBool(const char* lpszSection, const char* lpszKey, bool bDefault);
    //
    void __stdcall DumpToLog(void);
};

extern CConfig g_Config;

#endif  /* _CONFIG_H_ */
