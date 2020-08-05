
global  enable_irq
global  disable_irq

%include "int_const.inc"

[SECTION .text]

; ========================================================================
;                  void disable_irq(int irq);
; ========================================================================
; Disable an interrupt request line by setting an 8259 bit.
; Equivalent code:
;   if(irq < 8)
;       out_byte(INT_PORT_MASTER_DATA, in_byte(INT_PORT_MASTER_DATA) | (1 << irq));
;   else
;       out_byte(INT_PORT_SLAVE_DATA, in_byte(INT_PORT_SLAVE_DATA) | (1 << irq));
disable_irq:
    mov     ecx, [esp + 4]          ; irq
    pushf
    cli
    mov     ah, 1
    rol     ah, cl                  ; ah = (1 << (irq % 8))
    cmp     cl, 8
    jae     disable_8               ; disable irq >= 8 at the slave 8259
disable_0:
    in      al, INT_PORT_MASTER_DATA
    test    al, ah
    jnz     dis_already             ; already disabled?
    or      al, ah
    out     INT_PORT_MASTER_DATA, al       ; set bit at master 8259
    popf
    mov     eax, 1                  ; disabled by this function
    ret
disable_8:
    in      al, INT_PORT_SLAVE_DATA
    test    al, ah
    jnz     dis_already             ; already disabled?
    or      al, ah
    out     INT_PORT_SLAVE_DATA, al       ; set bit at slave 8259
    popf
    mov     eax, 1                  ; disabled by this function
    ret
dis_already:
    popf
    xor     eax, eax                ; already disabled
    ret

; ========================================================================
;                  void enable_irq(int irq);
; ========================================================================
; Enable an interrupt request line by clearing an 8259 bit.
; Equivalent code:
;       if(irq < 8)
;               out_byte(INT_PORT_MASTER_DATA, in_byte(INT_PORT_MASTER_DATA) & ~(1 << irq));
;       else
;               out_byte(INT_PORT_SLAVE_DATA, in_byte(INT_PORT_SLAVE_DATA) & ~(1 << irq));
;
enable_irq:
    mov     ecx, [esp + 4]          ; irq
    pushf
    cli
    mov     ah, ~1
    rol     ah, cl                  ; ah = ~(1 << (irq % 8))
    cmp     cl, 8
    jae     enable_8                ; enable irq >= 8 at the slave 8259
enable_0:
    in      al, INT_PORT_MASTER_DATA
    and     al, ah
    out     INT_PORT_MASTER_DATA, al       ; clear bit at master 8259
    popf
    ret
enable_8:
    ; in      al, INT_PORT_MASTER_DATA
    ; and     al, 0xFB
    ; out     INT_PORT_MASTER_DATA, al       ; clear bit 2 at master 8259
    in      al, INT_PORT_SLAVE_DATA
    and     al, ah
    out     INT_PORT_SLAVE_DATA, al       ; clear bit at slave 8259
    popf
    ret


