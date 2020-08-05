; 导入全局变量
extern  next_proc
extern  tss
extern  k_reenter ; 表示当前嵌套层数

global save
global restart
global restart_reenter

%include "proc.inc"

[SECTION .bss]
StackSpace   resb   2 * 1024
StackTop:   ; 这里(内核进程切换)使用的栈

[SECTION .text]
save:
    pushad
    push    ds
    push    es
    push    fs
    push    gs
    mov     dx, ss
    mov     ds, dx
    mov     es, dx

    ; esi = 进程表起始地址
    mov     esi, esp

    ; 每次save时使k_reenter++
    inc     dword [k_reenter]
    ; k_reenter == 0 切换到内核栈
    cmp     dword [k_reenter], 0
    jne     .1
    mov     esp, StackTop
    push    restart
    jmp     [esi + RETADDR - P_STACKBASE]
.1: 
    ; k_reenter > 0 已经在内核栈，不需要再切换
    push    restart_reenter
    jmp     [esi + RETADDR - P_STACKBASE]

restart:
    ; 把栈指针移到 next_proc 的位置
    mov esp, [next_proc]
    ; 加载下一个进程的 ldt
    lldt     [esp + P_LDT_SLT]
    ; 把next_proc->ldt_slt保存到tss.esp0
    lea eax, [esp + P_LDT_SLT]
    mov dword [tss + TSS_ESP0], eax

restart_reenter: ; 0x100c98
    ; 加载下一个进程的寄存器
    dec dword [k_reenter]
    pop gs
    pop fs
    pop es
    pop ds
    popad
    add esp, 4
    iretd
