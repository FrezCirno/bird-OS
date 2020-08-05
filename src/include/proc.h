#pragma once

#include <types.h> // u32
#include <protect.h> // DESCRIPTOR, SELECTOR

typedef struct s_stackframe
{
    u16 gs, _0;
    u16 fs, _1;
    u16 es, _2;
    u16 ds, _3;
    u32 edi;
    u32 esi;
    u32 ebp;
    u32 kernel_esp;
    u32 ebx;
    u32 edx;
    u32 ecx;
    u32 eax;
    u32 retaddr;
    u32 eip;
    u16 cs, _4;
    u32 eflags;
    u32 esp;
    u16 ss, _5;
} STACK_FRAME;

// 进程表/进程控制块
typedef struct s_proc
{
    STACK_FRAME regs; // 寄存器的值
    SELECTOR ldt_slt, _0;
    DESCRIPTOR ldt[LDT_SIZE];
    u32 pid;
    u8 name[16];
} PROCESS;

// 需要运行的任务数
#define NR_TASKS 3

// 进程管理, 定义于 sched.c
extern PROCESS  proc_table[NR_TASKS];
extern PROCESS* next_proc;
extern s32 k_reenter;

// 进程调度程序, restart.asm
void restart();

