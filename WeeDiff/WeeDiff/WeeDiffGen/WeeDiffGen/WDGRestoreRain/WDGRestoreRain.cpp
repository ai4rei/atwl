// -----------------------------------------------------------------
// WDGUseSSOLoginPacket
// (c) 2012-2013 Ai4rei/AN
//
// This work is licensed under a
// Creative Commons BY-NC-SA 3.0 Unported License
// http://creativecommons.org/licenses/by-nc-sa/3.0/
//
// -----------------------------------------------------------------

#include "WDGRestoreRain.h"

#ifndef _ARRAYSIZE
    #define _ARRAYSIZE(x) (sizeof(x)/sizeof((x)[0]))
#endif  /* _ARRAYSIZE */

static /* const */ WDGPLUGININFO l_PluginInfo =
{
    _T("Restore Rain"),
    _T("Restores 2004 clients' weather/map effect rain."),
    _T("[UI]"),
    _T(""),
    _T("Ai4rei/AN"),
    1,
    0,
    { 0x6161b1f3, 0xea19, 0x11e1, { 0xa7, 0x78, 0x0, 0x22, 0x15, 0x64, 0x8a, 0x98 } },  /* guid */
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
    UINT32 uOffset, uBOffset, uPart;

    this->m_DiffData.clear();

    if(!this->IsSane())
    {
        return NULL;
    }

    try
    {
        // MISSION: Dump rain code into .diff, fixup all offsets
        // according to CRagEffect::Snow and the like, and fixup
        // switch table offset for index 0xA1 (EF_RAIN).

        char szRainCode[] =
        /*000*/ "\xC8\x08\x00\x00"                           // ENTER   8,0                           ; 1:speed, 2:temp
        /*004*/ "\x56"                                       // PUSH    ESI
        /*005*/ "\x57"                                       // PUSH    EDI
        /*006*/ "\x53"                                       // PUSH    EBX
        /*007*/ "\x8B\xF1"                                   // MOV     ESI,ECX                       ; this (CRagEffect*)
        /*009*/ "\x33\xFF"                                   // XOR     EDI,EDI
        /*00B*/ "\x89\x7D\xFC"                               // MOV     DWORD PTR SS:[EBP-4],EDI      ; Local.1 = 0
        /*00E*/ "\xB9\x08\x59\x8F\x00"                       // MOV     ECX,OFFSET g_modeMgr          ; (abs fixup)
        /*013*/ "\xE8\xAB\x7D\xD3\xFF"                       // CALL    CModeMgr::GetGameMode         ; (rel fixup)
        /*018*/ "\x8B\x98\xD0\x00\x00\x00"                   // MOV     EBX,DWORD PTR DS:[EAX+D0]     ; m_world (abs fixup)
        /*01F*/ "\x8B\x4B\x3C"                               // MOV     ECX,DWORD PTR DS:[EBX+3C]     ; m_player (abs fixup)
        /*022*/ "\x8B\x41\x04"                               // MOV     EAX,DWORD PTR DS:[ECX+4]      ; this->m_pos = m_player->m_pos
        /*025*/ "\x89\x46\x04"                               // MOV     DWORD PTR DS:[ESI+4],EAX
        /*028*/ "\x8B\x41\x08"                               // MOV     EAX,DWORD PTR DS:[ECX+8]
        /*02B*/ "\x89\x46\x08"                               // MOV     DWORD PTR DS:[ESI+8],EAX
        /*02E*/ "\x8B\x41\x0C"                               // MOV     EAX,DWORD PTR DS:[ECX+C]
        /*031*/ "\x89\x46\x0C"                               // MOV     DWORD PTR DS:[ESI+C],EAX
        /*034*/ "\x8B\x86\x4C\x01\x00\x00"                   // MOV     EAX,DWORD PTR DS:[ESI+14C]    ; m_duration (abs fixup)
        /*03A*/ "\x2B\x86\x48\x01\x00\x00"                   // SUB     EAX,DWORD PTR DS:[ESI+148]    ; m_stateCnt (abs fixup)
        /*040*/ "\x83\xF8\x28"                               // CMP     EAX,28
        /*043*/ "\x0F\x8E\xFC\x00\x00\x00"                   // JLE     l_EOF
        /*049*/ "\x57"                                       // l_REDO: PUSH    EDI                   ; nullVector
        /*04A*/ "\x57"                                       // PUSH    EDI
        /*04B*/ "\x57"                                       // PUSH    EDI
        /*04C*/ "\x6A\x14"                                   // PUSH    14                            ; PP_3DLINE
        /*04E*/ "\x8B\xCE"                                   // MOV     ECX,ESI
        /*050*/ "\xE8\xAF\x1D\xE2\xFF"                       // CALL    CRagEffect::LaunchEffectPrim  ; (rel fixup)
        /*055*/ "\x89\x86\x50\x1C\x01\x00"                   // MOV     DWORD PTR DS:[ESI+11C50],EAX  ; m_prim (abs fixup)
        /*05B*/ "\x89\xB8\x58\x03\x00\x00"                   // MOV     DWORD PTR DS:[EAX+358],EDI    ; m_deltaPos2 = nullVector (abs fixup)
        /*061*/ "\x89\xB8\x5C\x03\x00\x00"                   // MOV     DWORD PTR DS:[EAX+35C],EDI    ; (abs fixup)
        /*067*/ "\x89\xB8\x60\x03\x00\x00"                   // MOV     DWORD PTR DS:[EAX+360],EDI    ; (abs fixup)
        /*06D*/ "\xC7\x80\xB0\x01\x00\x00\x28\x00\x00\x00"   // MOV     DWORD PTR DS:[EAX+1B0],28     ; m_duration (abs fixup)
        /*077*/ "\xC7\x80\x9C\x01\x00\x00\x14\x00\x00\x00"   // MOV     DWORD PTR DS:[EAX+19C],14     ; m_type (abs fixup)
        /*081*/ "\x8B\x96\x08\x01\x00\x00"                   // MOV     EDX,DWORD PTR DS:[ESI+108]    ; m_master (abs fixup)
        /*087*/ "\x8B\x4A\x04"                               // MOV     ECX,DWORD PTR DS:[EDX+4]      ; m_prim->m_pos = this->m_master->m_pos
        /*08A*/ "\x89\x48\x04"                               // MOV     DWORD PTR DS:[EAX+4],ECX
        /*08D*/ "\x8B\x4A\x08"                               // MOV     ECX,DWORD PTR DS:[EDX+8]
        /*090*/ "\x89\x48\x08"                               // MOV     DWORD PTR DS:[EAX+8],ECX
        /*093*/ "\x8B\x4A\x0C"                               // MOV     ECX,DWORD PTR DS:[EDX+C]
        /*096*/ "\x89\x48\x0C"                               // MOV     DWORD PTR DS:[EAX+C],ECX
        /*099*/ "\xFF\x70\x0C"                               // PUSH    DWORD PTR DS:[EAX+C]
        /*09C*/ "\xFF\x70\x04"                               // PUSH    DWORD PTR DS:[EAX+4]
        /*09F*/ "\x8B\x4B\x40"                               // MOV     ECX,DWORD PTR DS:[EBX+40]     ; (abs fixup)
        /*0A2*/ "\xE8\xFD\xBB\xBD\xFF"                       // CALL    C3dAttr::GetHeight            ; (rel fixup)
        /*0A7*/ "\xD8\x05\xD8\x4B\x85\x00"                   // FADD    DWORD PTR DS:[854BD8]         ; +200.0 (abs fixup)
        /*0AD*/ "\xD9\x5D\xF8"                               // FSTP    DWORD PTR SS:[EBP-8]          ; Local.2
        /*0B0*/ "\x8B\x86\x50\x1C\x01\x00"                   // MOV     EAX,DWORD PTR DS:[ESI+11C50]  ; m_prim (abs fixup)
        /*0B6*/ "\xD9\x40\x08"                               // FLD     DWORD PTR DS:[EAX+8]          ; m_pos.y
        /*0B9*/ "\xD8\x65\xF8"                               // FSUB    DWORD PTR SS:[EBP-8]          ; Local.2
        /*0BC*/ "\xD9\x58\x08"                               // FSTP    DWORD PTR DS:[EAX+8]          ; m_pos.y
        /*0BF*/ "\xE8\xE5\x92\x7C\x77"                       // CALL    rand                          ; (rel fixup)
        /*0C4*/ "\x99"                                       // CDQ
        /*0C5*/ "\xB9\xC8\x00\x00\x00"                       // MOV     ECX,0C8                       ; %200
        /*0CA*/ "\xF7\xF9"                                   // IDIV    ECX
        /*0CC*/ "\x83\xEA\x64"                               // SUB     EDX,64                        ; -100
        /*0CF*/ "\x89\x55\xF8"                               // MOV     DWORD PTR SS:[EBP-8],EDX      ; Local.2
        /*0D2*/ "\xDB\x45\xF8"                               // FILD    DWORD PTR SS:[EBP-8]          ; Local.2
        /*0D5*/ "\x8B\x86\x50\x1C\x01\x00"                   // MOV     EAX,DWORD PTR DS:[ESI+11C50]  ; m_prim (abs fixup)
        /*0DB*/ "\xD8\x40\x04"                               // FADD    DWORD PTR DS:[EAX+4]          ; m_pos.x
        /*0DE*/ "\xD9\x58\x04"                               // FSTP    DWORD PTR DS:[EAX+4]          ; m_pos.x
        /*0E1*/ "\xE8\xC3\x92\x7C\x77"                       // CALL    rand                          ; (rel fixup)
        /*0E6*/ "\x99"                                       // CDQ
        /*0E7*/ "\xB9\x64\x00\x00\x00"                       // MOV     ECX,64                        ; %100
        /*0EC*/ "\xF7\xF9"                                   // IDIV    ECX
        /*0EE*/ "\x83\xEA\x32"                               // SUB     EDX,32                        ; -50
        /*0F1*/ "\x89\x55\xF8"                               // MOV     DWORD PTR SS:[EBP-8],EDX      ; Local.2
        /*0F4*/ "\xDB\x45\xF8"                               // FILD    DWORD PTR SS:[EBP-8]          ; Local.2
        /*0F7*/ "\x8B\x86\x50\x1C\x01\x00"                   // MOV     EAX,DWORD PTR DS:[ESI+11C50]  ; m_prim (abs fixup)
        /*0FD*/ "\xD8\x40\x0C"                               // FADD    DWORD PTR DS:[EAX+C]          ; m_pos.z
        /*100*/ "\xD9\x58\x0C"                               // FSTP    DWORD PTR DS:[EAX+C]          ; m_pos.z
        /*103*/ "\x8B\x4B\x3C"                               // MOV     ECX,DWORD PTR DS:[EBX+3C]     ; m_player (abs fixup)
        /*106*/ "\x8B\x41\x04"                               // MOV     EAX,DWORD PTR DS:[ECX+4]      ; this->m_pos = m_player->m_pos
        /*109*/ "\x89\x46\x04"                               // MOV     DWORD PTR DS:[ESI+4],EAX
        /*10C*/ "\x8B\x41\x08"                               // MOV     EAX,DWORD PTR DS:[ECX+8]
        /*10F*/ "\x89\x46\x08"                               // MOV     DWORD PTR DS:[ESI+8],EAX
        /*112*/ "\x8B\x41\x0C"                               // MOV     EAX,DWORD PTR DS:[ECX+C]
        /*115*/ "\x89\x46\x0C"                               // MOV     DWORD PTR DS:[ESI+C],EAX
        /*118*/ "\x83\x7D\xFC\x00"                           // CMP     DWORD PTR SS:[EBP-4],0        ; Local.1
        /*11C*/ "\x0F\x8E\x23\x00\x00\x00"                   // JLE     l_EOF
        /*122*/ "\x8B\x86\x4C\x01\x00\x00"                   // MOV     EAX,DWORD PTR DS:[ESI+14C]    ; m_duration (abs fixup)
        /*128*/ "\x2B\x86\x48\x01\x00\x00"                   // SUB     EAX,DWORD PTR DS:[ESI+148]    ; m_stateCnt (abs fixup)
        /*12E*/ "\x3D\x2C\x01\x00\x00"                       // CMP     EAX,12C                       ; 300
        /*133*/ "\x0F\x8E\x0C\x00\x00\x00"                   // JLE     l_EOF
        /*139*/ "\xC7\x45\xFC\x00\x00\xA0\x40"               // MOV     DWORD PTR SS:[EBP-4],40A00000 ; Local.1 = 5.0
        /*140*/ "\xE9\x04\xFF\xFF\xFF"                       // JMP     l_REDO
        /*145*/ "\x5B"                                       // l_EOF:  POP     EBX
        /*146*/ "\x5F"                                       // POP     EDI
        /*147*/ "\x5E"                                       // POP     ESI
        /*148*/ "\xC9"                                       // LEAVE
        /*149*/ "\xC3"                                       // RETN
        /*14A*/ "\xCC"                                       // INT3
        /*14B*/ "\xCC"                                       // INT3
        /*14C*/ "\x00\x00\x48\x43"                           // DD      43480000                      ; 200.0f
        /*150*/ ;
    }
    catch(const char* lpszThrown)
    {
        char szErrMsg[1024];

        wsprintfA(szErrMsg, __FILE__" :: Part %u :: %s", uPart, lpszThrown);
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
