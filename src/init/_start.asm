extern init_gdt_idt, init_pic, setup_idt, init_tss ; main.c
extern gdt_ptr, idt_ptr ; protect.c
extern main ; sched.c
extern MemChkBuf ; memory.c

global _start

; 选择子
SELECTOR_DUMMY       equ   0
SELECTOR_FLAT_C      equ   0x08
SELECTOR_FLAT_RW     equ   0x10
SELECTOR_VIDEO       equ   (0x18 | 3) 
SELECTOR_TSS         equ   0x20
SELECTOR_LDT_FIRST   equ   0x28


[SECTION .text]

_start:
    ; 切换新栈
    ; 旧栈: 0x7c00
    mov esp, 0x7c00
    mov [MemChkBuf], ebx

    ; 切换GDT位置, 初始化中断和异常处理程序
    sgdt [gdt_ptr]

    call init_gdt_idt
    call setup_idt
    call init_tss
  
    lgdt [gdt_ptr]
    lidt [idt_ptr]

    jmp SELECTOR_FLAT_C:flush
flush:

    ; 加载 tr, 使之指向 GDT 中的 TSS 描述符
    xor eax, eax
    mov ax, SELECTOR_TSS
    ltr ax

    ; 进入c入口
    call main

; it seems no meaning
    sti
end:
    hlt
    jmp end 

