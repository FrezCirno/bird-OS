#pragma once
#include <bird/protect.h> // DESCRIPTOR, SELECTOR

typedef struct s_tss
{
    unsigned short prev __attribute__((aligned(4)));
    unsigned int esp0;
    unsigned short ss0 __attribute__((aligned(4)));
    unsigned int esp1;
    unsigned short ss1 __attribute__((aligned(4)));
    unsigned int esp2;
    unsigned short ss2 __attribute__((aligned(4)));
    unsigned int cr3;
    unsigned int eip;
    unsigned int flags;
    unsigned int eax;
    unsigned int ecx;
    unsigned int edx;
    unsigned int ebx;
    unsigned int esp;
    unsigned int ebp;
    unsigned int esi;
    unsigned int edi;
    unsigned short es __attribute__((aligned(4)));
    unsigned short cs __attribute__((aligned(4)));
    unsigned short ss __attribute__((aligned(4)));
    unsigned short ds __attribute__((aligned(4)));
    unsigned short fs __attribute__((aligned(4)));
    unsigned short gs __attribute__((aligned(4)));
    unsigned short ldt __attribute__((aligned(4)));
    /* I/O位图基址大于或等于TSS段界限，就表示没有I/O许可位图 */
    unsigned short trap, iobase;
} TSS;

typedef struct s_stackframe
{
    unsigned short gs __attribute__((aligned(4)));
    unsigned short fs __attribute__((aligned(4)));
    unsigned short es __attribute__((aligned(4)));
    unsigned short ds __attribute__((aligned(4)));
    unsigned int edi;
    unsigned int esi;
    unsigned int ebp;
    unsigned int kernel_esp;
    unsigned int ebx;
    unsigned int edx;
    unsigned int ecx;
    unsigned int eax;
    unsigned int retaddr;
    unsigned int eip;
    unsigned short cs __attribute__((aligned(4)));
    unsigned int eflags;
    unsigned int esp;
    unsigned short ss __attribute__((aligned(4)));
} STACK_FRAME;

// 进程表/进程控制块
typedef struct s_proc
{
    STACK_FRAME regs; // 寄存器的值
    SELECTOR ldt_slt __attribute__((aligned(4)));
    DESCRIPTOR ldt[LDT_SIZE];
    // 上面这几项不能变
    unsigned int pid;
    char name[16];
    int ticks; // 剩余时间片长度
    int priority;
    enum
    {
        RUNNING,
        SLEEPING
    } state;
} PROCESS;

typedef struct s_task
{
    void (*initial_eip)();
    int stacksize;
    char name[32];
} TASK;

// 需要运行的任务数
#define NR_TASKS 2 // 系统应用

#define NR_PROCTABLE (NR_TASKS)

// 进程管理, 定义于 proc.c
extern PROCESS proc_table[NR_PROCTABLE];
extern PROCESS *current;
extern TSS tss;
extern int k_reenter;

// 放弃保存状态, 直接切换到current -int_entry.asm
void restart();

// 保存当前进程状态, 切换到内核栈, 把返回地址设置成restart -int_entry.asm
void switch_to_current();

// 调度算法, 用于选择current
void schedule();

void *get_proc_laddr(int pid, void *offset);

void sleep_on(PROCESS **at);

void wake_up(PROCESS **at);
