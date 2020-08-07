#pragma once
#include <protect.h> // DESCRIPTOR, SELECTOR

typedef struct s_stackframe
{
    unsigned short gs, _0;
    unsigned short fs, _1;
    unsigned short es, _2;
    unsigned short ds, _3;
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
    unsigned short cs, _4;
    unsigned int eflags;
    unsigned int esp;
    unsigned short ss, _5;
} STACK_FRAME;

// 进程表/进程控制块
typedef struct s_proc
{
    STACK_FRAME regs; // 寄存器的值
    SELECTOR ldt_slt, _0;
    DESCRIPTOR ldt[LDT_SIZE];
    unsigned int pid;
    char name[16];
    int ticks;
    int priority;
} PROCESS;

typedef struct s_task
{
    void (*initial_eip)();
    int stacksize;
    char name[32];
} TASK;

// 需要运行的任务数
#define NR_TASKS 1 // 系统应用
#define NR_PROCS 3 // 用户程序

// 进程管理, 定义于 sched.c
extern PROCESS proc_table[];
extern PROCESS *next_proc;
extern int k_reenter;

// 进程调度程序, restart.asm
void restart();

void schedule();
