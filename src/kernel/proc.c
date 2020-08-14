#include <asm/io.h>
#include <bird/bird.h>
#include <bird/proc.h>
#include <bird/sys.h>
#include <stdio.h>
#include <glib.h>
#include <string.h>

PROCESS *current;
PROCESS proc_table[NR_PROCTABLE];
TSS tss;
int k_reenter;

void schedule()
{
    int greatest_ticks = 0;

start:
    // 检查是否有进程剩余时间片, 有则选择剩余时间片最大的进程放到下一个执行
    for (int i = 0; i < NR_PROCTABLE; i++)
    {
        PROCESS *p = &proc_table[i];
        if (p->state == RUNNING) // 不考虑被阻塞的进程
        {
            if (p->ticks > greatest_ticks)
            {
                greatest_ticks = p->ticks;
                current        = p;
            }
        }
    }

    // 如果所有进程都已用完时间片
    if (!greatest_ticks)
    {
        // 重置所有进程的时间片, 回到上一步
        for (int i = 0; i < NR_PROCTABLE; i++)
        {
            PROCESS *p = &proc_table[i];
            if (p->state == RUNNING) // 不考虑被阻塞的进程
            {
                p->ticks = p->priority;
            }
        }
        goto start;
    }
    fillRect(scr_x - 8, 0, scr_x, 16, PEN_BLACK);
    drawChar(scr_x - 8, 0, current->name[0], PEN_WHITE);
}

void *get_proc_laddr(int pid, void *offset)
{
    PROCESS *p = &proc_table[pid];

    unsigned int seg_base = BASE_OF_DESC(p->ldt[INDEX_LDT_FIRST]);
    unsigned int laddr    = seg_base + (unsigned int)offset;

    if (pid < NR_PROCTABLE) // 这里seg_base必定是0
    {
        assert(laddr == (unsigned int)offset);
    }

    return (void *)laddr;
}

void wake_up(PROCESS **queue)
{
    if (queue && *queue)
    {
        (**queue).state = RUNNING;
    }
}

// 在等待队列里排队
// 会把自己插到等待队列的开头, 并在被唤醒后依次唤醒之前的任务
void sleep_on(PROCESS **queue)
{
    PROCESS *prev;
    if (!queue) return;
    // 保存前面已经在等的任务
    prev = *queue;
    // 把自己放在队头
    *queue = current;
    // 进入休眠状态
    current->state = SLEEPING;
    printk("Process %s sleep...\n", PEN_RED, current->name);
    // 重新调度
    schedule();
    nothing(); // 利用系统调用重新调度
    // !!我被唤醒了, 依次唤醒前面的任务
    printk("Process %s wakeup...\n", PEN_RED, current->name);
    if (prev) prev->state = RUNNING;
}
