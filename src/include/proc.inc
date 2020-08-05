
; 进程 TCB 结构体, 偏移量必须和 proc.h 定义保持一致
P_STACKBASE     equ   0
REG_GS          equ   P_STACKBASE
REG_FS          equ   REG_GS         + 4
REG_ES          equ   REG_FS         + 4
REG_DS          equ   REG_ES         + 4
REG_EDI         equ   REG_DS         + 4
REG_ESI         equ   REG_EDI        + 4
REG_EBP         equ   REG_ESI        + 4
REG_KERNEL_ESP  equ   REG_EBP        + 4
REG_EBX         equ   REG_KERNEL_ESP + 4
REG_EDX         equ   REG_EBX        + 4
REG_ECX         equ   REG_EDX        + 4
REG_EAX         equ   REG_ECX        + 4
RETADDR         equ   REG_EAX        + 4
REG_EIP         equ   RETADDR        + 4
REG_CS          equ   REG_EIP        + 4
REG_EFLAGS      equ   REG_CS         + 4
REG_ESP         equ   REG_EFLAGS     + 4
REG_SS          equ   REG_ESP        + 4
P_LDT_SLT       equ   REG_SS         + 4
P_LDT           equ   P_LDT_SLT      + 4
TSS_ESP0        equ   4
