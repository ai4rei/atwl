// -----------------------------------------------------------------
// WDGEnableProxySupport
// (c) 2013 Ai4rei/AN
//
// This work is licensed under a
// Creative Commons BY-NC-SA 3.0 Unported License
// http://creativecommons.org/licenses/by-nc-sa/3.0/
//
// -----------------------------------------------------------------

#include "WDGEnableProxySupport.h"

#ifndef _ARRAYSIZE
    #define _ARRAYSIZE(x) (sizeof(x)/sizeof((x)[0]))
#endif  /* _ARRAYSIZE */

static /* const */ WDGPLUGININFO l_PluginInfo =
{
    _T("Enable Proxy Support"),
    _T("Ignores server-provided IP addresses when changing servers."),
    _T("[Fix]"),
    _T(""),
    _T("Ai4rei/AN"),
    1,
    0,
    { 0x6161b1f5, 0xea19, 0x11e1, { 0xa7, 0x78, 0x0, 0x22, 0x15, 0x64, 0x8a, 0x98 } },  /* guid */
    _T(""),
};

static WDGPlugin* l_lpSelfReference = NULL;

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
    FINDDATA Fd;
    UINT32 uOffset, uBOffset, uPart = 0;

    this->m_DiffData.clear();

    if(!this->IsSane())
    {
        return NULL;
    }

    try
    {
        // MISSION: Hijack connect() call in CConnection::Connect,
        // save the first IP that comes into sight and use it for
        // any following connection attempts.

        // Now since ws2_32::connect is linked by ordinal without
        // name, we cannot search for it with FindFunction, but
        // there is a certain string in CConnection::Connect, that
        // we can use instead...
        Fd.uMask = WFD_PATTERN|WFD_SECTION;
        Fd.lpData = "'Failed to setup select mode' 00";
        Fd.lpszSection = this->IsVC9Image() ? ".rdata" : ".data";
        uPart = 1;

        uOffset = this->m_dgc->Match(&Fd);

        // ...and function referencing it is CConnection::Connect.
        char cPushRef[5] = { 0x68 };
        Fd.uMask = WFD_SECTION;
        Fd.lpData = cPushRef;
        Fd.uDataSize = sizeof(cPushRef);
        Fd.lpszSection = ".text";
        ((UINT32*)&cPushRef[1])[0] = this->m_dgc->Raw2Rva(uOffset);
        uPart = 2;

        uOffset = this->m_dgc->Match(&Fd);

        bool bIndirectCALL = false;

        try
        {// VC9
            Fd.uMask = WFD_PATTERN|WFD_WILDCARD;
            Fd.lpData =
                /*00*/"FF15 '????'"  // CALL    NEAR DWORD PTR DS:[<&WS2_32.connect>]
                /*06*/"83F8 FF"      // CMP     EAX,-1
                /*09*/"75 '?'"       // JNZ     SHORT OFFSET v
                /*0B*/"8B3D '????'"  // MOV     EDI,DWORD PTR DS:[<&WS2_32.WSAGetLastError>]
                /*11*/"FFD7"         // CALL    NEAR EDI
                /*13*/"3D 33270000"  // CMP     EAX,2733h
                /*18*/;
            Fd.chWildCard = '?';
            Fd.uStart = uOffset-0x50;
            Fd.uFinish = uOffset;
            uPart = 3;

            uBOffset = this->m_dgc->Match(&Fd);

            this->SetByte(uBOffset++, 0x90);   // NOP
            bIndirectCALL = true;
        }
        catch(const char*)
        {// VC6
            Fd.uMask = WFD_PATTERN|WFD_WILDCARD;
            Fd.lpData =
                /*00*/"E8 '????'"    // CALL    <&WS2_32.connect>
                /*05*/"83F8 FF"      // CMP     EAX,-1
                /*08*/"75 '?'"       // JNZ     SHORT OFFSET v
                /*0A*/"E8 '????'"    // CALL    <&WS2_32.WSAGetLastError>
                /*0F*/"3D 33270000"  // CMP     EAX,2733h
                /*14*/;
            Fd.chWildCard = '?';
            Fd.uStart = uOffset-0x90;
            Fd.uFinish = uOffset;
            uPart = 4;

            uBOffset = this->m_dgc->Match(&Fd);
        }

        if(bIndirectCALL)
        {// VC9
            char cJmpCode[] =
            /*000*/ "\xA1\x00\x00\x00\x00"       // MOV     EAX,DWORD PTR DS:[<g_SaveIP>]
            /*005*/ "\x85\xC0"                   // TEST    EAX,EAX
            /*007*/ "\x75\x08"                   // JNZ     SHORT OFFSET v
            /*009*/ "\x8B\x46\x0C"               // MOV     EAX,DWORD PTR DS:[ESI+C]
            /*00C*/ "\xA3\x00\x00\x00\x00"       // MOV     DWORD PTR DS:[<g_SaveIP>],EAX
            /*011*/ "\x89\x46\x0C"               // MOV     DWORD PTR DS:[ESI+C],EAX
            /*014*/ "\xFF\x25\x00\x00\x00\x00"   // JMP     [OFFSET] ^
            /*01A*/ ;

            // get ourselves a space in .diff
            uPart = 5;
            uOffset = this->m_dgc->GetNextFreeOffset(0x4+0x1A);

            // g_SaveIP
            uPart = 6;
            ((UINT32*)&cJmpCode[0x01])[0] = this->m_dgc->Raw2Rva(uOffset);
            ((UINT32*)&cJmpCode[0x0D])[0] = this->m_dgc->Raw2Rva(uOffset);

            // WS2_32.connect address
            uPart = 7;
            ((UINT32*)&cJmpCode[0x16])[0] = this->m_dgc->GetDWORD32(uBOffset+1);

            // JMP in
            uPart = 8;
            this->SetByte(uBOffset+0, 0xE8);  // CALL
            this->SetLong(uBOffset+1, (this->m_dgc->Raw2Rva(uOffset)+4)-(this->m_dgc->Raw2Rva(uBOffset)+5));

            // Dump
            uPart = 9;
            this->SetLong(uOffset, 0x0);  // initial IP value (g_SaveIP)

            for(UINT32 uIdx = 0; uIdx<0x1AU; uIdx++)
            {
                this->SetByte(uOffset+0x4+uIdx, cJmpCode[uIdx]);
            }
        }
        else
        {// VC6
            char cJmpCode[] =
            /*000*/ "\xA1\x00\x00\x00\x00"       // MOV     EAX,DWORD PTR DS:[<g_SaveIP>]
            /*005*/ "\x85\xC0"                   // TEST    EAX,EAX
            /*007*/ "\x75\x08"                   // JNZ     SHORT OFFSET v
            /*009*/ "\x8B\x46\x0C"               // MOV     EAX,DWORD PTR DS:[ESI+C]
            /*00C*/ "\xA3\x00\x00\x00\x00"       // MOV     DWORD PTR DS:[<g_SaveIP>],EAX
            /*011*/ "\x89\x46\x0C"               // MOV     DWORD PTR DS:[ESI+C],EAX
            /*014*/ "\xE9\x00\x00\x00\x00"       // JMP     OFFSET ^
            /*019*/ ;

            // get ourselves a space in .diff
            uPart = 5;
            uOffset = this->m_dgc->GetNextFreeOffset(0x4+0x19);

            // g_SaveIP
            uPart = 6;
            ((UINT32*)&cJmpCode[0x01])[0] = this->m_dgc->Raw2Rva(uOffset);
            ((UINT32*)&cJmpCode[0x0D])[0] = this->m_dgc->Raw2Rva(uOffset);

            // WS2_32.connect address
            uPart = 7;
            ((UINT32*)&cJmpCode[0x15])[0] = ((this->m_dgc->Raw2Rva(uBOffset)+5)+(INT32)this->m_dgc->GetDWORD32(uBOffset+1))-(this->m_dgc->Raw2Rva(uOffset)+0x1D);

            // JMP in
            uPart = 8;
            this->SetByte(uBOffset+0, 0xE9);  // JMP
            this->SetLong(uBOffset+1, (this->m_dgc->Raw2Rva(uOffset)+4)-(this->m_dgc->Raw2Rva(uBOffset)+5));

            // Dump
            uPart = 9;
            this->SetLong(uOffset, 0x0);  // initial IP value (g_SaveIP)

            for(UINT32 uIdx = 0; uIdx<0x19U; uIdx++)
            {
                this->SetByte(uOffset+0x4+uIdx, cJmpCode[uIdx]);
            }
        }
    }
    catch(const char* lpszThrown)
    {
        char szErrMsg[1024];

        wsprintfA(szErrMsg, __FILE__" :: Part %u :: %s", uPart, lpszThrown);
        this->m_dgc->LogMsg(szErrMsg);

        // clean up diffdata (half diff)
        this->m_DiffData.clear();
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

void WDGPlugin::SetLong(UINT32 uOffset, UINT32 uValue)
{
    UCHAR* lpucValue = (UCHAR*)&uValue;

    for(UINT32 i = 0; i < 4; i++)
    {
        this->SetByte(uOffset+i, lpucValue[i]);
    }
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
