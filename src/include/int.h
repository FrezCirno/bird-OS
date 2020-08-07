#pragma once

/* 中断向量表 */
// 硬件异常
#define INT_VECTOR_DIVIDE       0x0
#define INT_VECTOR_DEBUG        0x1
#define INT_VECTOR_NMI          0x2
#define INT_VECTOR_BREAKPOINT   0x3
#define INT_VECTOR_OVERFLOW     0x4
#define INT_VECTOR_BOUNDS       0x5
#define INT_VECTOR_INVAL_OP     0x6
#define INT_VECTOR_COPROC_NOT   0x7
#define INT_VECTOR_DOUBLE_FAULT 0x8
#define INT_VECTOR_COPROC_SEG   0x9
#define INT_VECTOR_INVAL_TSS    0xA
#define INT_VECTOR_SEG_NOT      0xB
#define INT_VECTOR_STACK_FAULT  0xC
#define INT_VECTOR_PROTECTION   0xD
#define INT_VECTOR_PAGE_FAULT   0xE
#define INT_VECTOR_COPROC_ERR   0x10
// 外部中断, 从8259A传到CPU
#define INT_VECTOR_IRQ          0x20
#define INT_VECTOR_IRQ_CLOCK    0x0
#define INT_VECTOR_IRQ_KEYBOARD 0x1
#define INT_VECTOR_IRQ_2        0x2
#define INT_VECTOR_IRQ_COM2     0x3
#define INT_VECTOR_IRQ_COM1     0x4
#define INT_VECTOR_IRQ_LPT2     0x5
#define INT_VECTOR_IRQ_FDD      0x6
#define INT_VECTOR_IRQ_LPT1     0x7
#define INT_VECTOR_IRQ_RTC      0x8
#define INT_VECTOR_IRQ_REDRT    0x9
#define INT_VECTOR_IRQ_MOUSE    0xC
#define INT_VECTOR_IRQ_FPU      0xD
#define INT_VECTOR_IRQ_AT       0xE
// 软件中断, 由 int 指令产生
#define INT_VECTOR_SYS_CALL 0x90

#define NR_IRQ      16
#define NR_SYS_CALL 10

typedef void (*int_handler)();
typedef void (*irq_handler)(unsigned int);

extern irq_handler irq_table[NR_IRQ];
extern void *syscall_table[NR_SYS_CALL];

// int_ctl.c
void disable_irq(unsigned int irq);
void enable_irq(unsigned int irq);

// exceptionh.c
void exception_handler(unsigned int vec_no, unsigned int err_code,
                       unsigned int eip, unsigned int cs, unsigned int eflags);

// irqh.c
void default_irq_handler(unsigned int irq);
void put_irq_handler(unsigned int irq, irq_handler handler);

// syscallh.c
int sys_getticks();
int sys_nothing();
