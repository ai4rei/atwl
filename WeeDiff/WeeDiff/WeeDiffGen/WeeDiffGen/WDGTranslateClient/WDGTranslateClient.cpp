// -----------------------------------------------------------------
// WDGTranslateClient
// (c) 2012 Ai4rei/AN
//
// This work is licensed under a
// Creative Commons BY-NC-SA 3.0 Unported License
// http://creativecommons.org/licenses/by-nc-sa/3.0/
//
// -----------------------------------------------------------------

#include "WDGTranslateClient.h"

#ifndef _ARRAYSIZE
    #define _ARRAYSIZE(x) (sizeof(x)/sizeof((x)[0]))
#endif  /* _ARRAYSIZE */

//#define ___VERBOSE

static /* const */ WDGPLUGININFO l_PluginInfo =
{
    _T("Translate Client"),
    _T("Translates the client with strings stored in WDGTranslateClient.txt"),
    _T("[UI]"),
    _T(""),
    _T("Ai4rei/AN"),
    1,
    0,
    { 0x6161b1eb, 0xea19, 0x11e1, { 0xa7, 0x78, 0x0, 0x22, 0x15, 0x64, 0x8a, 0x98 } },  /* guid */
    _T(""),
};

static WDGPlugin* l_lpSelfReference = NULL;
TCHAR l_szLocation[MAX_PATH] = { 0 };

void WDGPlugin::Release(void)
{
    this->m_DiffData.clear();
    l_lpSelfReference = NULL;
    delete this;
}

void WDGPlugin::Free(LPVOID lpBuf)
{
    delete lpBuf;
}

LPWDGPLUGININFO WDGPlugin::GetPluginInfo(void)
{
    return &l_PluginInfo;
}

INT32 WDGPlugin::Enabled(void)
{
    return 0;
}

INT32 WDGPlugin::Disabled(void)
{
    return 0;
}

LPCTSTR WDGPlugin::GetInputValue(void)
{
    return NULL;
}

DiffData* WDGPlugin::GeneratePatch(void)
{
    UINT32 uPart = 0;

    this->m_DiffData.clear();

    if(!this->IsSane())
    {
        return NULL;
    }

    try
    {
        // MISSION: Load WDGTranslateClient.txt, parse it and find+
        // replace the contained string pairs.
        CXLateBE XLateBE(l_szLocation);

        if(!XLateBE.Count())
        {
            throw "Nothing to translate.";
        }

        uPart = 1;

        XLateBE.ForEach(&ApplyTranslation, this);
    }
    catch(const char* lpszThrown)
    {
        char szErrMsg[1024];

        wsprintfA(szErrMsg, __FILE__" :: Part %u :: %s", uPart, lpszThrown);
        this->m_dgc->LogMsg(szErrMsg);

        // clean up diffdata (half diff)
        this->m_DiffData.clear();
    }
    catch(UINT32 uThrowLine)
    {
        char szErrMsg[1024];

        wsprintfA(szErrMsg, __FILE__" :: Part %u :: Parse error in %ls on line %u", uPart, l_szLocation, uThrowLine);
        this->m_dgc->LogMsg(szErrMsg);
    }
    catch(TLITEM* lpItem)
    {
        char szErrMsg[1024];

        wsprintfA(szErrMsg, __FILE__" :: Part %u :: Translation '%s' could not be fit into '%s' (see example for max. lengths)", uPart, lpItem->second.c_str(), lpItem->first.c_str());
        this->m_dgc->LogMsg(szErrMsg);
    }

    return this->m_DiffData.empty() ? NULL : &this->m_DiffData;
}

DiffData* WDGPlugin::GetDiffData(void)
{
    return this->m_DiffData.empty() ? NULL : &this->m_DiffData;
}

bool WDGPlugin::IsVC9Image(void)
{
    try
    {
        this->m_dgc->FindFunction("_encode_pointer");  // msvcr90.dll
    }
    catch(const char* lpszThrown)
    {
        return false;

        // unused
        (void)lpszThrown;
    }

    return true;
}

bool WDGPlugin::IsSane(void)
{// ensure that it actually is a valid RO client
#define ISSANEMAGIC "gravity"
    FINDDATA Fd;

    Fd.uMask       = WFD_SECTION;
    Fd.lpData      = ISSANEMAGIC;
    Fd.uDataSize   = sizeof(ISSANEMAGIC);
    Fd.lpszSection = this->IsVC9Image() ? ".rdata" : ".data";

    try
    {
        this->m_dgc->Match(&Fd);
    }
    catch(const char* lpszThrown)
    {
        this->m_dgc->LogMsg(__FILE__" :: Image is not sane.");
        return false;

        // unused
        (void)lpszThrown;
    }

    return true;
#undef ISSANEMAGIC
}


void WDGPlugin::SetByte(UINT32 uOffset, UCHAR uValue)
{
    DIFFDATA Diff = { uOffset, uValue };

    this->m_DiffData.push_back(Diff);
}

void WDGPlugin::SetBuffer(UINT uOffset, CHAR* lpBuffer, UINT32 uSize)
{
    for(UINT32 uIdx = 0; uIdx<uSize; uIdx++)
    {
        this->SetByte(uOffset++, lpBuffer[uIdx]);
    }
}

bool WDGPlugin::TryMatch(LPFINDDATA lpFd, UINT32* lpuOffset)
{
    try
    {
        lpuOffset[0] = this->m_dgc->Match(lpFd);
    }
    catch(const char* lpszThrown)
    {
        return false;

        // unused
        (void)lpszThrown;
    }

    return true;
}

bool CALLBACK WDGPlugin::ApplyTranslation(TLITEM* lpItem, void* lpContext)
{
    bool bSuccess = false;
#ifdef ___VERBOSE
    char szErrMsg[1024];
#endif  /* ___VERBOSE */
    FINDDATA Fd = { 0 };
    UINT32 uOffset = 0;
    WeeUtility::HEXBUFFER hbMatch, hbPaste;
    WDGPlugin* lpThis = (WDGPlugin*)lpContext;

    hbMatch = WeeUtility::HexStr2Buffer(lpItem->first.c_str());
    hbPaste = WeeUtility::HexStr2Buffer(lpItem->second.c_str());

    // check whether paste fits into aligned match...
    if(hbMatch.uSize+((4-hbMatch.uSize)&3)<hbPaste.uSize)
    {
        throw lpItem;
    }

    Fd.uMask = WFD_SECTION;
    Fd.lpData = (CHAR*)hbMatch.buffer;
    Fd.uDataSize = hbMatch.uSize;
    Fd.lpszSection = lpThis->IsVC9Image() ? ".rdata" : ".data";

    if(lpThis->TryMatch(&Fd, &uOffset))
    {
        lpThis->SetBuffer(uOffset, (CHAR*)hbPaste.buffer, hbPaste.uSize);
        bSuccess = true;
#ifdef ___VERBOSE
        wsprintfA(szErrMsg, __FILE__" :: DEBUG :: Translation '%s'->'%s' OK.", lpItem->first.c_str(), lpItem->second.c_str());
        lpThis->m_dgc->LogMsg(szErrMsg);
    }
    else
    {
        wsprintfA(szErrMsg, __FILE__" :: DEBUG :: Translation '%s'->'%s' FAILED.", lpItem->first.c_str(), lpItem->second.c_str());
        lpThis->m_dgc->LogMsg(szErrMsg);
#endif  /* ___VERBOSE */
    }

    delete[] hbPaste.buffer;
    delete[] hbMatch.buffer;
    return bSuccess;
}

extern "C" __declspec(dllexport) WeeDiffGenPlugin::IWDGPlugin* InitPlugin(LPVOID lpData, USHORT huWeeDiffMajorVersion, USHORT huWeeDiffMinorVersion)
{
    if(l_lpSelfReference)
    {// double initialization
        DebugBreak();
    }
    else
    {
        l_lpSelfReference = new WDGPlugin(lpData);
    }

    return l_lpSelfReference;
}

extern "C" BOOL CALLBACK DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
    switch(dwReason)
    {
        case DLL_PROCESS_ATTACH:
            DisableThreadLibraryCalls(hInstance);
            GetModuleFileName(hInstance, l_szLocation, _ARRAYSIZE(l_szLocation));
            _tcscpy(_tcsrchr(l_szLocation, '.'), _T(".txt"));  // TODO: Make this safer.
            break;
    }

    return TRUE;
}
