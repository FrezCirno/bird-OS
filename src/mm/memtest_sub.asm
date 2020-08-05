global memtest_sub

memtest_sub:    ; u32 memtest_sub(u32 start, u32 end)
    PUSH    EDI                     ; （由于还要使用EBX, ESI, EDI）
    PUSH    ESI
    PUSH    EBX
    MOV     ESI,0xaa55aa55          ; pat0 = 0xaa55aa55;
    MOV     EDI,0x55aa55aa          ; pat1 = 0x55aa55aa;
    MOV     EAX,[ESP+12+4]          ; i = start;
mts_loop:
    MOV     EBX,EAX
    ADD     EBX,0xffc               ; p = i + 0xffc;
    MOV     EDX,[EBX]               ; old = *p;
    MOV     [EBX],ESI               ; *p = pat0;
    XOR     DWORD [EBX],0xffffffff  ; *p ^= 0xffffffff;
    CMP     EDI,[EBX]               ; if (*p != pat1) goto fin;
    JNE     mts_fin
    XOR     DWORD [EBX],0xffffffff  ; *p ^= 0xffffffff;
    CMP     ESI,[EBX]               ; if (*p != pat0) goto fin;
    JNE     mts_fin
    MOV     [EBX],EDX               ; *p = old;
    ADD     EAX,0x1000              ; i += 0x1000;
    CMP     EAX,[ESP+12+8]          ; if (i <= end) goto mts_loop;
    JBE     mts_loop
    POP     EBX
    POP     ESI
    POP     EDI
    RET
mts_fin:
    MOV     [EBX],EDX               ; *p = old;
    POP     EBX
    POP     ESI
    POP     EDI
    RET