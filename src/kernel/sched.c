#include <types.h>
#include <glib.h>    // drawText
#include <string.h>  // memcpy
#include <proc.h>    // NR_TASKS, PROCESS
#include <syscall.h> // syscall
#include <memory.h>  // memtest
#include <clock.h>
#include <keyboard.h>
#include <mouse.h>

PROCESS *next_proc;

PROCESS proc_table[NR_TASKS];

s32 k_reenter;

// 需要执行的任务

void fooA();
void fooB();
void fooC();

typedef struct s_task
{
    void (*initial_eip)();
    int stacksize;
    char name[32];
} TASK;

/* stacks of tasks */
#define STACK_SIZE_TESTA 0x8000
#define STACK_SIZE_TESTB 0x8000
#define STACK_SIZE_TESTC 0x8000

#define STACK_SIZE_TOTAL \
    (STACK_SIZE_TESTA + STACK_SIZE_TESTB + STACK_SIZE_TESTC)

char task_stack[STACK_SIZE_TOTAL]; // 测试程序ABC公用的栈

TASK task_table[NR_TASKS] = {{fooA, STACK_SIZE_TESTA, "TestA"},
                             {fooB, STACK_SIZE_TESTB, "TestB"},
                             {fooC, STACK_SIZE_TESTC, "TestC"}};
// 以上, 需要执行的任务

void main() // 0x100da9
{
    u32 memtotal = mm_init();

    init_video();
    printstr("Memtest: ", PEN_BLUE);
    printstr(itoa(memtotal, 10), PEN_BLUE);
    printstr(" KB\n", PEN_BLUE);

    init_clock();
    init_keyboard();
    init_mouse();

    TASK *pTask      = task_table;
    PROCESS *pProc   = proc_table;
    char *pTaskStack = task_stack + STACK_SIZE_TOTAL;
    SELECTOR ldt_slt = SELECTOR_LDT_FIRST;

    for (int i = 0; i < NR_TASKS; i++)
    {
        pProc->pid = i;                   // pid
        strcpy(pProc->name, pTask->name); // name of the process

        pProc->ldt_slt =
            ldt_slt; // 选择子只要指定 LDT 描述符在  GDT 中的地址就够了

        // 初始化 GDT 中的 LDT 描述符
        set_desc(&gdt[pProc->ldt_slt >> 3],
                 vir2phys(seg2phys(SELECTOR_KERNEL_DS), pProc->ldt),
                 LDT_SIZE * sizeof(DESCRIPTOR) - 1, DA_P | DA_LDT);

        // 把GDT的代码段描述符拷贝到进程LDT[0]里
        memcpy(&pProc->ldt[0], &gdt[INDEX_FLAT_C], sizeof(DESCRIPTOR));
        pProc->ldt[0].attr1 = DA_P | DA_CXO | RPL_TASK << 5;
        // 把GDT的数据段描述符拷贝到进程LDT[1]里
        memcpy(&pProc->ldt[1], &gdt[INDEX_FLAT_RW], sizeof(DESCRIPTOR));
        pProc->ldt[1].attr1 = DA_P | DA_DRW | RPL_TASK << 5;
        // 段选择子(因为使用LDT故设置TI位)
        pProc->regs.cs     = (8 * 0) | SA_TI | RPL_TASK;
        pProc->regs.ds     = (8 * 1) | SA_TI | RPL_TASK;
        pProc->regs.es     = (8 * 1) | SA_TI | RPL_TASK;
        pProc->regs.fs     = (8 * 1) | SA_TI | RPL_TASK;
        pProc->regs.ss     = (8 * 1) | SA_TI | RPL_TASK;
        pProc->regs.gs     = SELECTOR_KERNEL_GS | RPL_TASK;
        pProc->regs.eip    = (u32)pTask->initial_eip;
        pProc->regs.esp    = (u32)pTaskStack;
        pProc->regs.eflags = 0x1202; /* IF=1, IOPL=1 */

        pTaskStack -= pTask->stacksize;
        pProc++;
        pTask++;
        ldt_slt += 8;
    }

    k_reenter = 0;

    next_proc = &proc_table[0];

    restart();

    for (;;)
        ;
}

// 以下是测试程序ABC
void delay()
{
    for (int i = 0; i < 100000; i++)
        ;
}

void fooA()
{
    while (1)
    {
        keyboard_read();
    }
}

void fooB()
{
    while (1)
    {
        mouse_read();
    }
}

void fooC()
{
    while (1)
    {
        delay();
    }
}