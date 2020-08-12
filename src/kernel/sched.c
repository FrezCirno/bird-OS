#include <bird/proc.h>   // NR_TASKS, PROCESS
#include <bird/memory.h> // memtest
#include <bird/bird.h>   // panic
#include <string.h>      // memcpy
#include <glib.h>        // drawText
#include <int.h>
#include <clock.h>
#include <keyboard.h>
#include <mouse.h>
#include <hd.h>

void tty();
void read();
void init();
void fooC();

/* stacks of tasks */
#define STACK_SIZE_FOO   0x800
#define STACK_SIZE_TOTAL 0x8000

char task_stack[STACK_SIZE_TOTAL]; // 测试程序ABC公用的栈

// init必须放第一个, 因为要画背景
TASK task_table[NR_TASKS] = {{init, STACK_SIZE_FOO, "init"},
                             {read, STACK_SIZE_FOO, "read"}};

void main()
{
    init_pic();
    unsigned int memtotal = mm_init();
    init_video();
    init_clock();
    init_keyboard();
    init_mouse();
    init_hd();

    TASK *pTask      = task_table;
    PROCESS *pProc   = proc_table;
    char *pTaskStack = task_stack + STACK_SIZE_TOTAL;
    SELECTOR ldt_slt = SELECTOR_LDT_FIRST;

    unsigned char rpl;
    int eflags;

    for (int i = 0; i < NR_TASKS; i++)
    {
        pTask  = task_table + i;
        rpl    = RPL_USER;
        eflags = EFLAG_IOPL1 | EFLAG_IOPL2 | EFLAG_IF | EFLAG_RES;

        pProc->pid      = i; // pid
        pProc->state    = RUNNING;
        pProc->ticks    = 5;
        pProc->priority = 5;
        strcpy(pProc->name, pTask->name); // name of the process

        // 选择子只要指定 LDT 描述符在  GDT 中的地址就够了
        pProc->ldt_slt = ldt_slt;

        // 初始化 GDT 中的 LDT 描述符
        set_seg_desc(&gdt[pProc->ldt_slt >> 3],
                     vir2phys(gdt_desc_base(SELECTOR_KERNEL_DS), pProc->ldt),
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

    k_reenter = 0;

    current = &proc_table[0];

    restart();

    panic("This will never be executed!");
}

// 以下是测试程序ABC
#define delay() for (int i = 0; i < 1000000; i++)

// 根进程, 类似explorer, 负责绘制整个背景, 和fork出新的程序
void init()
{
    // 默认颜色背景, 没有这个会留残影
    SHEET *bg = alloc_sheet();
    sheet_setbuf(bg, (unsigned char *)mm_alloc_4k(scr_x * scr_y), scr_x, scr_y,
                 -1);
    memset(bg->buf, PEN_BLACK, scr_x * scr_y);
    movez(bg, 0);

    SHEET *sht = alloc_sheet();
    sheet_setbuf(sht, mm_alloc_4k(320 * 200), 320, 200, 0);
    movexy(sht, 100, 200);
    movez(sht, ctl->top);

    unsigned char mbr[512] = {0, 1, 2,  3,  4,  5,  6,  7,
                              8, 9, 10, 11, 12, 13, 14, 15};

    for (int i = 0; i < 512; i++) mbr[i] = 0xff;
    rw_abs_hd(1, 0, 0, 1, mbr);
    for (int i = 0; i < 512; i++) mbr[i] = 0x0;
    rw_abs_hd(0, 0, 0, 1, mbr);

    int i = 0;
    while (1)
    {
        // printstr("i", PEN_LIGHT_YELLOW);
        drawWindowTo(sht->buf, sht->bxsize, 320, 200, "init");
        for (int j = 0; j < 512; j++)
        {
            int x = 4 + (j * fonts.Width) % sht->bxsize;
            int y = 24 + (j * fonts.Width) / sht->bxsize * fonts.Height;
            drawRectTo(sht->buf, sht->bxsize, x, y, 5 * fonts.Width,
                       fonts.Height, PEN_LIGHT_YELLOW);
            drawTextTo(sht->buf, sht->bxsize, x, y, itoa(mbr[j], 16), PEN_BLACK);
        }

        refresh_local(sht, 0, 0, 320, 200);
        i++;
    }

    // int pid = fork();
    // if (pid != 0) // parent
    // {
    //     int stat;
    //     int child = wait(&stat);
    // }
    // else // child
    // {
    //     getpid();
    //     execl();
    // }
}

void read()
{
    while (1)
    {
        mouse_read();
        keyboard_read();
    }
}

void tty()
{
    unsigned char mbr[512];

    SHEET *sht = alloc_sheet();
    sheet_setbuf(sht, mm_alloc_4k(320 * 200), 320, 200, 0);
    // rw_abs_hd(0, 0, 0, 1, mbr);

    while (1)
    {
        printstr("t", PEN_WHITE);
        for (int i = 0; i < 512; i++)
        {
            // printstr(itoa(mbr[i], 10), PEN_BLUE);
        }
        delay();
    }
}

void fooC()
{
    SHEET *sht = alloc_sheet();
    sheet_setbuf(sht, mm_alloc_4k(320 * 200), 320, 200, -1);
    movexy(sht, 400, 600);
    movez(sht, ctl->top);
    drawWindowTo(sht->buf, sht->bxsize, 320, 200, "fooC");

    int i = 0;
    while (1)
    {
        printstr("c", PEN_LIGHT_YELLOW);
        fillRectTo(sht->buf, sht->bxsize, 0, 0, 10 * fonts.Width, fonts.Height,
                   PEN_LIGHT_GRAY);
        drawTextTo(sht->buf, sht->bxsize, 0, 0, itoa(i, 10), PEN_LIGHT_BLUE);
        refresh_local(sht, 0, 0, sht->bxsize, sht->bysize);
        i++;
    }
}