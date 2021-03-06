// -----------------------------------------------------------------
// WDGAllowUnknownSlashCommands
// (c) 2021 Ai4rei/AN
//
// This work is licensed under a
// Creative Commons BY-NC-SA 4.0 International License
// https://creativecommons.org/licenses/by-nc-sa/4.0/
//
// -----------------------------------------------------------------

#include "WDGAllowUnknownSlashCommands.h"

#ifndef _ARRAYSIZE
    #define _ARRAYSIZE(x) (sizeof(x)/sizeof((x)[0]))
#endif  /* _ARRAYSIZE */

static /* const */ WDGPLUGININFO l_PluginInfo =
{
    _T("Allow unknown /commands"),
    _T("Instead of showing \"Invalid command\", unknown /commands are passed to the server as regular text, to allow server-side /commands."),
    _T("[UI]"),
    _T(""),
    _T("Ai4rei/AN"),
    1,
    0,
    { 0x017FF2E0, 0x7E09, 0x11EB, { 0xA7, 0x8B, 0x74, 0xDA, 0x38, 0xC2, 0x2B, 0x45 } },  /* {017FF2E0-7E09-11eb-A78B-74DA38C22B45} */
    _T(""),
};

static WDGPlugin* l_lpSelfReference = NULL;

void WDGPlugin::Release()
{
    m_DiffData.clear();
    l_lpSelfReference = NULL;
    delete this;
}

void WDGPlugin::Free(LPVOID lpBuf)
{
    delete lpBuf;
}

LPWDGPLUGININFO WDGPlugin::GetPluginInfo()
{
    return &l_PluginInfo;
}

INT32 WDGPlugin::Enabled()
{
    return 0;
}

INT32 WDGPlugin::Disabled()
{
    return 0;
}

LPCTSTR WDGPlugin::GetInputValue()
{
    return NULL;
}

DiffData* WDGPlugin::GeneratePatch()
{
    FINDDATA Fd;
    UINT32 uPart;

    m_DiffData.clear();

    try
    {
        // MISSION: Turn TT_UNKNOWN return value into TT_NORMAL in
        // function CSession::GetNoParamTalkType and in caller
        // (CSession::GetTalkType) set chatStartOffset to 0 instead
        // of -1 in case the return value was indeed TT_NORMAL.

        // 20150916aRagexe ~ 20190107aRagexe. This generic code
        // sequence is actually unique in the client, so use a
        // simple FR.
        Fd.uMask = WFD_PATTERN;
        Fd.uStart = 0;
        Fd.uFinish = (UINT32)-1;
        Fd.lpData = "75 09"         // JNZ      <+9>
                    "8B40 04"       // MOV      EAX,[EAX+4]
                    "8BE5"          // MOV      ESP,EBP
                    "5D"            // POP      EBP
                    "C2 0400"       // RETN     4
                    "B8 03000000"   // MOV      EAX,3       ; TT_UNKNOWN
                    "8B E5"         // MOV      ESP,EBP
                    "5D"            // POP      EBP
                    "C2 0400"       // RETN     4
                    ;

        uPart = 1;
        return GeneratePatchV1(m_dgc->Match(&Fd));
    }
    catch(LPCSTR)
    {
        try
        {
            // 20120702aRagexeRE
            Fd.uMask = WFD_PATTERN;
            Fd.uStart = 0;
            Fd.uFinish = (UINT32)-1;
            Fd.lpData = "5F"            // POP      EDI
                        "5E"            // POP      ESI
                        "59"            // POP      ECX
                        "C2 0400"       // RETN     4
                        "5F"            // POP      EDI
                        "B8 03000000"   // MOV      EAX,3
                        "5E"            // POP      ESI
                        "59"            // POP      ECX
                        "C2 0400"       // RETN     4
                        ;

            uPart = 7;
            return GeneratePatchV2(m_dgc->Match(&Fd));
        }
        catch(LPCSTR lpszThrown)
        {
            char szErrMsg[1024];
            wsprintfA(szErrMsg, __FILE__" :: Part %u :: %s", uPart, lpszThrown);
            m_dgc->LogMsg(szErrMsg);
        }
    }

    return NULL;
}

DiffData* WDGPlugin::GeneratePatchV1(UINT32 uOffset)
{
    FINDDATA Fd;
    UINT32 uBegin, uPart;
    UINT32 uBeginRva, uCallerRva, uRelativeCall;
    UINT32 uUJmpRva, uUJmpTrmp, uUJmpTrmpRel, uUJmp;
    UINT32 uCJmpRva, uCJmpTrmp, uCJmpTrmpRel;

    try
    {
        // MOV EAX,3
        SetByte(uOffset+11, 0x33);  // -> XOR EAX,EAX (TT_NORMAL = 0)
        SetByte(uOffset+12, 0xC0);
        SetByte(uOffset+13, 0x83);  // -> ADD EDI,1 (EDI is 0 at this point in the caller)
        SetByte(uOffset+14, 0xC7);
        SetByte(uOffset+15, 0x01);

        // Find (trace backwards) prologue of the current function
        // to obtain its offset (PUSH EBP; MOVE EBP,ESP).
        for(uBegin = uOffset; uBegin>0 && !(m_dgc->GetBYTE(uBegin)==0x55 && m_dgc->GetWORD(uBegin+1)==0xEC8B); uBegin--);
        if(uBegin==0)
        {
            uPart = 2;
            throw "Begin of function not found, function signature has probably changed.";
        }

        // Get the actual address.
        uBeginRva = m_dgc->Raw2Rva(uBegin);

        // Find the only reference to this function (CALL).
        Fd.uMask = WFD_PATTERN;
        Fd.uStart = 0;
        Fd.uFinish = (UINT32)-1;
        Fd.lpData = "8B00"          // MOV      EAX,[EAX]
                    "8B4D 9C"       // MOV      ECX,[LOCAL.25]
                    "50"            // PUSH     EAX
                    "E8"            // CALL     <+/->           ; NOTE: this is the only thing that is actually needed, the above stuff is just to speed the search up a bit
                    ;
        uPart = 3;

        for(;;)
        {
            // Since the built-in Matches() facility has no context
            // parameter, we have to roll our own.
            uOffset = m_dgc->Match(&Fd);

            // Calculate the CALL argument
            uCallerRva = m_dgc->Raw2Rva(uOffset+11);  // EIP after the call
            uRelativeCall = uBeginRva-uCallerRva;     // CALL offset

            if(m_dgc->GetDWORD32(uOffset+7)==uRelativeCall)
            {// found
                break;
            }

            Fd.uStart = uOffset+1;  // try next
        }

        // Unconditional JMP destination
        uUJmpRva = m_dgc->GetWORD(uOffset+14)+m_dgc->Raw2Rva(uOffset+18);
        uUJmp = m_dgc->Rva2Raw(uUJmpRva);

        if(m_dgc->GetWORD(uUJmp)!=0xCF83 || m_dgc->GetBYTE(uUJmp+2)!=0xFF)
        {// not OR EDI,-1
            uPart = 4;
            throw "Unexpected instruction found.";
        }

        // Find unconditional trampoline +/-80h for UJmp (JMP SHORT)
        Fd.uMask = WFD_PATTERN|WFD_WILDCARD;
        Fd.uStart = uOffset+18-0x80;
        Fd.uFinish = uOffset+18+0x80+4;
        Fd.chWildCard = '?';
        Fd.lpData = "E9 '??' 0000";  // JMP <+>
        uPart = 5;

        for(;;)
        {
            uUJmpTrmp = m_dgc->Match(&Fd);

            // do not match the JMP we are about to replace
            if(uUJmpTrmp!=uOffset+13)
            {
                uCallerRva = m_dgc->Raw2Rva(uUJmpTrmp+5);
                uRelativeCall = uUJmpRva-uCallerRva;

                if(m_dgc->GetDWORD32(uUJmpTrmp+1)==uRelativeCall)
                {// found
                    uUJmpTrmpRel = m_dgc->Raw2Rva(uUJmpTrmp)-m_dgc->Raw2Rva(uOffset+18);
                    break;
                }
            }

            Fd.uStart = uUJmpTrmp+1;
        }

        // Conditional JMP destination (is just three bytes after that)
        uCJmpRva = uUJmpRva+3;

        // Find unconditional trampoline +/-80h for CJmp (JZ SHORT)
        Fd.uMask = WFD_PATTERN|WFD_WILDCARD;
        Fd.uStart = uOffset+16-0x80;
        Fd.uFinish = uOffset+16+0x80+4;
        Fd.chWildCard = '?';
        Fd.lpData = "E9 '??' 0000";  // JMP <+>
        uPart = 6;

        for(;;)
        {
            uCJmpTrmp = m_dgc->Match(&Fd);

            uCallerRva = m_dgc->Raw2Rva(uCJmpTrmp+5);
            uRelativeCall = uCJmpRva-uCallerRva;

            if(m_dgc->GetDWORD32(uCJmpTrmp+1)==uRelativeCall)
            {// found
                uCJmpTrmpRel = m_dgc->Raw2Rva(uCJmpTrmp)-m_dgc->Raw2Rva(uOffset+16);
                break;
            }

            Fd.uStart = uCJmpTrmp+1;
        }

        // JMP <+/->
        SetByte(uOffset+13, 0x4F);  // -> DEC EDI (for TT_NORMAL this makes it 0, otherwise -1 (because there was no previous increment and by default EDI is zero in this function)
        SetByte(uOffset+14, 0x74);  // -> JZ ...
        SetByte(uOffset+15, (UCHAR)uCJmpTrmpRel);  // -> ... <+/->
        SetByte(uOffset+16, 0xEB);  // -> JMP SHORT ...
        SetByte(uOffset+17, (UCHAR)uUJmpTrmpRel);  // -> ... <+/->
    }
    catch(const char* lpszThrown)
    {
        char szErrMsg[1024];

        wsprintfA(szErrMsg, __FILE__" :: Part %u :: %s", uPart, lpszThrown);
        m_dgc->LogMsg(szErrMsg);

        // clean up diffdata (half diff)
        m_DiffData.clear();
    }

    return m_DiffData.empty() ? NULL : &m_DiffData;
}

DiffData* WDGPlugin::GeneratePatchV2(UINT32 uOffset)
{
    FINDDATA Fd;
    UINT32 uBegin, uPart, uMatches = 0;
    UINT32 uBeginRva, uCallerRva, uRelativeCall;

    try
    {
        // MOV EAX,3
        SetByte(uOffset+7, 0x33);  // -> XOR EAX,EAX (TT_NORMAL = 0)
        SetByte(uOffset+8, 0xC0);
        SetByte(uOffset+9, 0x83);  // -> ADD EBP,1 (EBP is 0 at this point in the caller)
        SetByte(uOffset+10, 0xC5);
        SetByte(uOffset+11, 0x01);

        // Find begin of the function (INT3, PUSH ECX, PUSH ESI).
        for(uBegin = uOffset; uBegin>0 && !(m_dgc->GetBYTE(uBegin)==0xCC && m_dgc->GetWORD(uBegin+1)==0x5651); uBegin--);
        if(uBegin==0)
        {
            uPart = 8;
            throw "Begin of function not found, function signature has probably changed.";
        }

        // Get the actual address.
        uBeginRva = m_dgc->Raw2Rva(uBegin+1);

        // Find the two references to this function (CALL).
        Fd.uMask = WFD_PATTERN|WFD_WILDCARD;
        Fd.uStart = 0;
        Fd.uFinish = (UINT32)-1;
        Fd.chWildCard = '?';
        Fd.lpData = "E8 '????'"     // CALL     <+/->
                    "8907"          // MOV      [EDI],EAX
                    "83CD FF"       // OR       EBP,-1
                    ;
        uPart = 9;

        for(;;)
        {
            uOffset = m_dgc->Match(&Fd);

            // Calculate the CALL argument
            uCallerRva = m_dgc->Raw2Rva(uOffset+5);   // EIP after the call
            uRelativeCall = uBeginRva-uCallerRva;     // CALL offset

            if(m_dgc->GetDWORD32(uOffset+1)==uRelativeCall)
            {// found
                uMatches++;
                uPart++;

                SetByte(uOffset+8, 0xED);   // OR EBP,-1 -> SUB EBP,1 (for TT_NORMAL, EBP becomes 0, otherwise -1)
                SetByte(uOffset+9, 0x01);

                if(uMatches==2)
                {// done
                    break;
                }
            }

            Fd.uStart = uOffset+1;  // try next
        }
    }
    catch(const char* lpszThrown)
    {
        char szErrMsg[1024];

        wsprintfA(szErrMsg, __FILE__" :: Part %u :: %s", uPart, lpszThrown);
        m_dgc->LogMsg(szErrMsg);

        // clean up diffdata (half diff)
        m_DiffData.clear();
    }

    return m_DiffData.empty() ? NULL : &m_DiffData;
}

DiffData* WDGPlugin::GetDiffData(void)
{
    return m_DiffData.empty() ? NULL : &m_DiffData;
}

void WDGPlugin::SetByte(UINT32 uOffset, UCHAR uValue)
{
    DIFFDATA Diff = { uOffset, uValue };

    m_DiffData.push_back(Diff);
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
