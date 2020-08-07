#include <bird/gui.h>    // drawText
#include <bird/proc.h>   // NR_TASKS, PROCESS
#include <bird/memory.h> // memtest
#include <string.h>      // memcpy
#include <clock.h>
#include <keyboard.h>
#include <mouse.h>

PROCESS *next_proc;

PROCESS proc_table[NR_TASKS + NR_PROCS];

int k_reenter;

void tty();
void fooA();
void fooB();
void fooC();

/* stacks of tasks */
#define STACK_SIZE_FOO   0x800
#define STACK_SIZE_TOTAL 0x8000

char task_stack[STACK_SIZE_TOTAL]; // 测试程序ABC公用的栈

TASK task_table[NR_TASKS] = {{tty, STACK_SIZE_FOO, "tty"}};

TASK user_proc_table[NR_PROCS] = {{fooA, STACK_SIZE_FOO, "TestA"},
                                  {fooB, STACK_SIZE_FOO, "TestB"},
                                  {fooC, STACK_SIZE_FOO, "TestC"}};

void main()
{
    unsigned int memtotal = mm_init();
    init_video();
    init_clock();
    init_keyboard();
    init_mouse();

    TASK *pTask      = task_table;
    PROCESS *pProc   = proc_table;
    char *pTaskStack = task_stack + STACK_SIZE_TOTAL;
    SELECTOR ldt_slt = SELECTOR_LDT_FIRST;

    unsigned char privilege;
    unsigned char rpl;
    int eflags;

    for (int i = 0; i < NR_TASKS + NR_PROCS; i++)
    {
        if (i < NR_TASKS)
        {
            pTask     = task_table + i;
            privilege = RPL_TASK;
            rpl       = RPL_TASK;
            eflags    = 0x1202; /* IF=1, IOPL=1 */
        }
        else
        {
            pTask     = user_proc_table + i - NR_TASKS;
            privilege = RPL_TASK;
            rpl       = RPL_TASK;
            eflags    = 0x1202;
        }

        pProc->pid = i;                   // pid
        strcpy(pProc->name, pTask->name); // name of the process

        // 选择子只要指定 LDT 描述符在  GDT 中的地址就够了
        pProc->ldt_slt = ldt_slt;

        // 初始化 GDT 中的 LDT 描述符
        set_seg_desc(&gdt[pProc->ldt_slt >> 3],
                     vir2phys(seg2phys(SELECTOR_KERNEL_DS), pProc->ldt),
                     LDT_SIZE * sizeof(DESCRIPTOR) - 1, DA_P | DA_LDT);

        // 把GDT的代码段描述符拷贝到进程LDT[0]里
        memcpy(&pProc->ldt[0], &gdt[INDEX_FLAT_C], sizeof(DESCRIPTOR));
        pProc->ldt[0].attr1 = DA_P | DA_CXO | rpl << 5;
        // 把GDT的数据段描述符拷贝到进程LDT[1]里
        memcpy(&pProc->ldt[1], &gdt[INDEX_FLAT_RW], sizeof(DESCRIPTOR));
        pProc->ldt[1].attr1 = DA_P | DA_DRW | rpl << 5;

        // 段选择子(因为使用LDT故设置TI位)
        pProc->regs.cs     = (8 * 0) | SA_TI | rpl;
        pProc->regs.ds     = (8 * 1) | SA_TI | rpl;
        pProc->regs.es     = (8 * 1) | SA_TI | rpl;
        pProc->regs.fs     = (8 * 1) | SA_TI | rpl;
        pProc->regs.ss     = (8 * 1) | SA_TI | rpl;
        pProc->regs.gs     = SELECTOR_KERNEL_GS | rpl;
        pProc->regs.eip    = (unsigned int)pTask->initial_eip;
        pProc->regs.esp    = (unsigned int)pTaskStack;
        pProc->regs.eflags = eflags;

        pTaskStack -= pTask->stacksize;
        pProc++;
        pTask++;
        ldt_slt += 8;
    }

    proc_table[0].ticks = proc_table[0].priority = 5;
    proc_table[1].ticks = proc_table[1].priority = 5;
    proc_table[2].ticks = proc_table[2].priority = 5;
    proc_table[3].ticks = proc_table[3].priority = 5;

    k_reenter = 0;

    next_proc = &proc_table[0];

    restart();

    for (;;)
        ;
}

void schedule()
{
    int greatest_ticks = 0;

    while (!greatest_ticks)
    {
        for (PROCESS *p = proc_table; p < proc_table + NR_TASKS + NR_PROCS; p++)
        {
            if (p->ticks > greatest_ticks)
            {
                greatest_ticks = p->ticks;
                next_proc      = p;
            }
        }

        if (!greatest_ticks)
        {
            for (PROCESS *p = proc_table; p < proc_table + NR_TASKS + NR_PROCS;
                 p++)
            {
                p->ticks = p->priority;
            }
        }
    }
}

// 以下是测试程序ABC
void delay()
{
    for (int i = 0; i < 100000; i++)
        ;
}

void tty()
{
    int a = 0;

    WINDOW *win = createWindow(120, 200, 320, 200, "tty", 0);
    while (1)
    {
        a++;
        fillRectTo(win->body.buf, win->body.bxsize, 0, 0, 15 * fonts.Width,
                   fonts.Height, PEN_LIGHT_GRAY);
        drawTextTo(win->body.buf, win->body.bxsize, 0, 0, itoa(a, 10),
                   PEN_LIGHT_BLUE);
        refresh_local(win->sht, 0, 0, win->sht->bxsize, win->sht->bysize);
        delay();
    }
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
    WINDOW *win = createWindow(120, 120, 160, 68, "fooC", 0);

    int i = 0;
    while (1)
    {
        fillRectTo(win->body.buf, win->body.bxsize, 0, 0, 15 * fonts.Width,
                   fonts.Height, PEN_LIGHT_GRAY);
        drawTextTo(win->body.buf, win->body.bxsize, 0, 0, itoa(i, 10),
                   PEN_LIGHT_BLUE);
        refresh_local(win->sht, 0, 0, win->sht->bxsize, win->sht->bysize);
        i++;
    }
}