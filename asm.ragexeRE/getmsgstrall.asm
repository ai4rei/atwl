    ENTER   0,0                                                        ;  // Dumps msgstringtable.txt [0;722[
    PUSH    szModeWrite                                                ;  ASCII "w"
    PUSH    szMsgStringTable                                           ;  ASCII "msgStringTable.txt"
    CALL    MSVCRT.fopen
    ADD     ESP,8h
    TEST    EAX,EAX
    JE      SHORT lError
    PUSH    EDI
    MOV     EDI,EAX
    PUSH    ESI
    XOR     ESI,ESI
lContinue:
    PUSH    ESI
    CALL    MsgStr
    PUSH    EAX
    PUSH    OFFSET szStrLF                                             ;  ASCII "%s\n"
    PUSH    EDI
    CALL    MSVCRT.fprintf
    ADD     ESP,10h
    INC     ESI
    CMP     ESI,722h
    JL      lContinue
    PUSH    EDI
    CALL    MSVCRT.fclose
    ADD     ESP,4h
    POP     ESI
    POP     EDI
lError:
    LEAVE
    RETN

; Base 00738CBC
; C8 00 00 00 68 30 9C 76 00 68 10 B1 78 00 E8 0E 6A 8E 77 83 C4 08 85 C0 74 2F 57 8B F8 56 33 F6
; 56 E8 DE 9A E4 FF 50 68 A4 0B 77 00 57 E8 8C 3C 8D 77 83 C4 0C 46 81 FE 22 07 00 00 7C E2 57 E8
; EE 46 8D 77 83 C4 04 5E 5F C9 C3
