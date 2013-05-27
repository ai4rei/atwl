// -----------------------------------------------------------------
// WDGTranslateClient
// (c) 2012 Ai4rei/AN
//
// This work is licensed under a
// Creative Commons BY-NC-SA 3.0 Unported License
// http://creativecommons.org/licenses/by-nc-sa/3.0/
//
// -----------------------------------------------------------------

#ifndef _XLATEBE_H_
#define _XLATEBE_H_

#ifndef tstring
    #ifdef UNICODE
        #define tstring wstring
    #else  /* UNICODE */
        #define tstring string
    #endif  /* UNICODE */
#endif  /* tstring */

typedef std::pair<std::string, std::string> TLITEM;
typedef bool (CALLBACK* LPFNXLATEBECALLBACK)(TLITEM* lpItem, void* lpContext);

class CXLateBE
{
    // i like pair-variable names to be same length, but i could not
    // come up with better equivalents for find/replace, sorry ;;
    enum { ST_MATCH, ST_PASTE };
    typedef std::vector<TLITEM> TLITEMS;

    std::vector<TLITEM> m_Translation;

protected:
    static char* RightTrimLF(char* lpszString)
    {
        UINT32 uLen = strlen(lpszString);

        if(uLen)
        {
            while(--uLen && (lpszString[uLen]=='\n' || lpszString[uLen]=='\r'))
            {
                lpszString[uLen] = 0;
            }
        }

        return lpszString;
    }

public:
    CXLateBE(std::tstring sFileName)
    {
        char szLine[0x1000];  // should be sufficient
        UINT32 uLine = 0, uState = ST_MATCH;
        FILE* hFile;
        std::string sMatch, sPaste;

        _tfopen_s(&hFile, sFileName.c_str(), _T("r"));

        if(!hFile)
        {
            throw "CXLateBE::CXLateBE: File not found.";
        }

        while(fgets(szLine, _ARRAYSIZE(szLine), hFile))
        {
            uLine++;

            if(szLine[0]==0x00 || szLine[0]=='\n' || szLine[0]=='\r')
            {// empty line
                ;
            }
            else switch(((WORD*)&szLine[0])[0])
            {
                case '/'|('/'<<8):
                    // comment
                    break;
                case 'F'|(':'<<8):
                    // find
                    if(uState!=ST_MATCH)
                    {
                        fclose(hFile);
                        throw uLine;
                    }

                    uState = ST_PASTE;
                    sMatch = RightTrimLF(&szLine[2]);
                    break;
                case 'R'|(':'<<8):
                    // replace
                    if(uState!=ST_PASTE)
                    {
                        fclose(hFile);
                        throw uLine;
                    }

                    uState = ST_MATCH;
                    sPaste = RightTrimLF(&szLine[2]);

                    m_Translation.push_back(TLITEM(sMatch+"00", sPaste+"00"));
                    break;
                default:
                    fclose(hFile);
                    throw uLine;
            }
        }

        fclose(hFile);
    }
    //
    UINT32 ForEach(LPFNXLATEBECALLBACK lpFunc, void* lpContext)
    {
        UINT32 uIdx, uCount = 0;

        for(uIdx = 0; uIdx<m_Translation.size(); uIdx++)
        {
            if(lpFunc(&m_Translation[uIdx], lpContext))
            {
                uCount++;
            }
        }

        return uCount;
    }
    UINT32 Count()
    {
        return m_Translation.size();
    }
};

#endif  /* _XLATEBE_H_ */
