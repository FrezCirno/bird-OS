; 中断处理程序, c语言
extern   exception_handler ; exceptionh.c
extern   irq_table ; irqh.c
extern   syscall_table ; syscall_entryh.c
extern   k_reenter ; 表示当前嵌套层数
extern   current ; proc.h
extern   tss  ; protect.h

global   restart
global   switch_to_current

; 硬件异常处理入口 void int_handler();
global   divide_error
global   single_step_exception
global   nmi
global   breakpoint_exception
global   overflow
global   bounds_check
global   inval_opcode
global   copr_not_available
global   double_fault
global   copr_seg_overrun
global   inval_tss
global   segment_not_present
global   stack_exception
global   general_protection
global   page_fault
global   copr_error
; 外部中断处理入口 从8259A进入CPU
global   hwint00
global   hwint01
global   hwint02
global   hwint03
global   hwint04
global   hwint05
global   hwint06
global   hwint07
global   hwint08
global   hwint09
global   hwint10
global   hwint11
global   hwint12
global   hwint13
global   hwint14
global   hwint15
global   syscall_entry


INT_PORT_MASTER_CMD    equ   0x20
INT_PORT_MASTER_DATA   equ   0x21
INT_PORT_SLAVE_CMD     equ   0xA0
INT_PORT_SLAVE_DATA    equ   0xA1
INT_EOI                equ   0x20


; 进程 TCB 结构体, 偏移量必须和 proc.h 定义保持一致
P_STACKBASE     equ   0
; STACK_FRAME regs
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
P_STACKTOP      equ   REG_SS         + 4
; SELECTOR ldt_slt
P_LDT_SLT       equ   REG_SS         + 4
; DESCRIPTOR ldt[LDT_SIZE]
P_LDT           equ   P_LDT_SLT      + 4
TSS_ESP0        equ   4


[SECTION .bss]
StackSpace   resb   2 * 1024
StackTop:   ; 这里(内核进程切换)使用的栈

[SECTION .text]

; 异常处理入口
divide_error:
    push 0xFFFFFFFF ; no err code
    push 0  ; vector_no = 0
    jmp exception

single_step_exception:
    push 0xFFFFFFFF ; no err code
    push 1  ; vector_no = 1
    jmp exception

nmi:
    push 0xFFFFFFFF ; no err code
    push 2  ; vector_no = 2
    jmp exception

breakpoint_exception:
    push 0xFFFFFFFF ; no err code
    push 3  ; vector_no = 3
    jmp exception

overflow:
    push 0xFFFFFFFF ; no err code
    push 4  ; vector_no = 4
    jmp exception

bounds_check:
    push 0xFFFFFFFF ; no err code
    push 5  ; vector_no = 5
    jmp exception

inval_opcode:
    push 0xFFFFFFFF ; no err code
    push 6  ; vector_no = 6
    jmp exception

copr_not_available:
    push 0xFFFFFFFF ; no err code
    push 7  ; vector_no = 7
    jmp exception

double_fault:
    push 8  ; vector_no = 8
    jmp exception

copr_seg_overrun:
    push 0xFFFFFFFF ; no err code
    push 9  ; vector_no = 9
    jmp exception

inval_tss:
    push 10  ; vector_no = A
    jmp exception

segment_not_present:
    push 11  ; vector_no = B
    jmp exception

stack_exception:
    push 12  ; vector_no = C
    jmp exception

general_protection:
    push 13  ; vector_no = D
    jmp exception

page_fault:
    push 14  ; vector_no = E
    jmp exception

copr_error:
    push 0xFFFFFFFF ; no err code
    push 16  ; vector_no = 10h
    jmp exception

exception:
    ; 0x100624
    call exception_handler
    add esp, 4*2 ; 让栈顶指向 EIP，堆栈中从顶向下依次是：EIP、CS、EFLAGS
    hlt


; 中断处理入口
; ---------------------------------
%macro  hwint_master    1
    call save
    ; 屏蔽当前中断
    in  al, INT_PORT_MASTER_DATA
    or  al, (1 << %1) 
    out INT_PORT_MASTER_DATA, al
    ; 置EOI位
    mov al, INT_EOI
    out INT_PORT_MASTER_CMD, al
    sti ; CPU在响应中断的过程中会自动关中断，这句之后就允许响应新的中断
    push %1
    call [irq_table + 4 * %1]
    pop ecx
    cli
    ; 恢复接受当前中断
    in  al, INT_PORT_MASTER_DATA
    and al, ~(1 << %1) 
    out INT_PORT_MASTER_DATA, al
    ret
%endmacro
; ---------------------------------

ALIGN 16
hwint00:         ; Interrupt routine for irq 0 (the clock).
    hwint_master    0

ALIGN 16
hwint01:         ; Interrupt routine for irq 1 (keyboard)
    hwint_master    1

ALIGN 16
hwint02:         ; Interrupt routine for irq 2 (cascade!)
    hwint_master    2

ALIGN 16
hwint03:         ; Interrupt routine for irq 3 (second serial)
    hwint_master    3

ALIGN 16
hwint04:         ; Interrupt routine for irq 4 (first serial)
    hwint_master    4

ALIGN 16
hwint05:         ; Interrupt routine for irq 5 (XT winchester)
    hwint_master    5

ALIGN 16
hwint06:         ; Interrupt routine for irq 6 (floppy)
    hwint_master    6

ALIGN 16
hwint07:         ; Interrupt routine for irq 7 (printer)
    hwint_master    7

; ---------------------------------
%macro  hwint_slave     1
    call save
    ; 屏蔽当前中断
    in  al, INT_PORT_SLAVE_DATA
    or  al, (1 << (%1 - 8)) 
    out INT_PORT_SLAVE_DATA, al
    ; 置EOI位 x2
    mov al, INT_EOI
    out INT_PORT_SLAVE_CMD, al ; 先次PIC
    nop
    out INT_PORT_MASTER_CMD, al ; 后主PIC
    nop
    sti ; CPU在响应中断的过程中会自动关中断，这句之后就允许响应新的中断
    push %1
    call [irq_table + 4 * %1]
    pop ecx
    cli 
    ; 恢复接受当前中断
    in  al, INT_PORT_SLAVE_DATA
    and al, ~(1 << (%1 - 8)) 
    out INT_PORT_SLAVE_DATA, al
    ret
%endmacro
; ---------------------------------

ALIGN 16
hwint08:         ; Interrupt routine for irq 8 (realtime clock).
    hwint_slave     8

ALIGN 16
hwint09:         ; Interrupt routine for irq 9 (irq 2 redirected)
    hwint_slave     9

ALIGN 16
hwint10:         ; Interrupt routine for irq 10
    hwint_slave     10

ALIGN 16
hwint11:         ; Interrupt routine for irq 11
    hwint_slave     11

ALIGN 16
hwint12:         ; Interrupt routine for irq 12
    hwint_slave     12

ALIGN 16
hwint13:         ; Interrupt routine for irq 13 (FPU exception)
    hwint_slave     13

ALIGN 16
hwint14:         ; Interrupt routine for irq 14 (AT winchester)
    hwint_slave     14

ALIGN 16
hwint15:         ; Interrupt routine for irq 15
    hwint_slave     15


ALIGN 16
syscall_entry:
    call save ; 调用后esi指向proc_table中当前正在运行的进程项, esp->内核栈(栈顶是restart/restart_reenter地址)
    sti
    push edx ; 系统调用参数3
    push ecx ; 系统调用参数2
    push ebx ; 系统调用参数1
    call [syscall_table + eax * 4] ; 这里用到了eax
    add esp, 12
    mov [esi + REG_EAX], eax ; 返回值插到旧状态的eax里面
    cli
    ret ; 此处返回会进入restart/restart_reenter, esi指向proc_table中当前正在运行的进程项, esp->内核栈


; 发生中断时
; 无特权级变化: 栈中依次是eip, cs, eflags
; 
; 有特权级变化(必然, 因为中断处理程序是系统代码):
;  因为这里的特权级为0, 所以CPU会自动切换到内核栈(tss.esp0), 它已经被预设为指向proc_table中对应项的regs成员尾地址,
;  然后向新栈中压入程序状态, 栈内: eip, cs, eflags, old_esp, old_ss
;  正好装进了proc_table对应项的regs成员中, 实现进程的固定


; 手动模拟中断过程, 除了特权级没变
; ; 不行, 特权级不变遇到lldt就会GP!!
_switch  db  0,0
         db  0,0
switch_to_current:
    mov [_switch], esp
    ; 切换栈到当前进程的保存地
    mov esp, dword [tss + TSS_ESP0]
    push ss
    push dword [_switch] ; 此进程再次运行时的栈, 栈中是call的返回地址
    pushfd
    push cs
    push dword .1 ; 此进程再次运行时的eip
    call save
.1:
    ret ; 这里被使用两次


; 在旧栈(指向proc_table)里面保存所有状态, 由esi指向
; esp切换到内核栈, 并修改调用者的返回地址为restart/restart_reenter
save:
    ; 在中断处理程序中调用, 当前特权级应该是0
    ; 现在esp/栈应该指向proc_table中当前正在执行的进程项(的中间位置), 
    ;  栈中: caller_eip(call的返回地址), eip, cs, eflags, old_esp, old_ss
    pushad
    push ds
    push es
    push fs
    push gs
    ; 当前栈: gs, fs, es, ds, edi, esi, ebp, esp, ebx, edx, ecx, eax, 
    ; 正好凑成一个TCB

    ; 反正esi会更新, 这里借用一下
    mov si, ss
    mov ds, si
    mov es, si

    ; esi = 凑成的TCB的起始地址
    mov esi, esp

    inc dword [k_reenter]
    cmp dword [k_reenter], 0
    jne .alreadykernel
    ; 切换到内核栈
    mov esp, StackTop
    push restart
    jmp [esi + RETADDR] ; call this返回, 调用本函数的函数返回时将会进入restart

.alreadykernel:
    ; 已经在内核栈，不需要再切换
    push restart_reenter
    jmp [esi + RETADDR] ; call this返回, 调用本函数的函数返回时将会进入restart_reenter


restart:
    ; 此时esi指向proc_table中前一个进程的位置, esp指向内核栈(空)
    ; 把current装到esp中(注意栈, 注意栈与PROCESS结构体的对应)
    mov esp, [current]
    ; 加载current->ldt_slt
    lldt     [esp + P_LDT_SLT] 
    ; 把current->regs的末地址保存到tss.esp0, 以备下次中断时本进程的状态可以保存到对应位置
    lea eax, [esp + P_STACKTOP]
    mov dword [tss + TSS_ESP0], eax

restart_reenter:
    dec dword [k_reenter]
    ; 加载current进程的寄存器状态
    pop gs
    pop fs
    pop es
    pop ds
    popad
    add esp, 4 ; 跳过ret_addr
    ; 现在栈中依次是eip, cs, eflags, old_esp, old_ss
    ; 返回到current进程继续执行, 相当于一次普通的中断返回
    iretd