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

SHEET *win;

void main() // 0x100da9
{
    u32 memtotal = mm_init();
    init_video();
    init_clock();
    init_keyboard();
    init_mouse();

    win = sheet_alloc();
    sheet_setbuf(win, (u8 *)mm_alloc_4k(160 * 68), 160, 68, -1);
    drawWindowTo(win->buf, win->bxsize, win->bxsize, win->bysize, "Hello world!");

    drawTextTo(win->buf, win->bxsize, 24, 28, "Memtest:     KB", PEN_BLUE);
    drawTextTo(win->buf, win->bxsize, 24, 28 + 9 * fonts.Width,
               itoa(memtotal, 10), PEN_BLUE);

    sheet_updown(win, 1);
    sheet_slide(win, 80, 72);

    TASK *pTask      = task_table;
    PROCESS *pProc   = proc_table;
    char *pTaskStack = task_stack + STACK_SIZE_TOTAL;
    SELECTOR ldt_slt = SELECTOR_LDT_FIRST;

    for (int i = 0; i < NR_TASKS; i++)
    {
        pProc->pid = i;                   // pid
        strcpy(pProc->name, pTask->name); // name of the process

        // 选择子只要指定 LDT 描述符在  GDT 中的地址就够了
        pProc->ldt_slt = ldt_slt;

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
    int a = 0;
    while (1)
    {
        a++;
        fillRectTo(win->buf, win->bxsize, 20, 20, 60, 36, PEN_LIGHT_GRAY);
        drawTextTo(win->buf, win->bxsize, 20, 20, itoa(a, 10), PEN_BLUE);
        sheet_refresh_sheet(win, 20, 20, 120, 44);
        delay();
    }
}