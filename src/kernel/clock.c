#include <types.h>
#include <int.h>   // put_irq_handler
#include <proc.h>  // next_proc
#include <clock.h> // declarations

int ticks;

// INT_VECTOR_IRQ_CLOCK 修改ticks
// INT_VECTOR_SYS_CALL + 0 获取ticks
void init_clock()
{
    ticks = 0;
    put_irq_handler(INT_VECTOR_IRQ_CLOCK, clock_handler);
    enable_irq(INT_VECTOR_IRQ_CLOCK);
}

// 时钟中断处理器, 主要任务是计时和调度程序
void clock_handler(u32 irq)
{
    ticks++;
    if (k_reenter != 0)
    {
        return;
    }

    next_proc++;
    if (next_proc == proc_table + NR_TASKS)
    {
        next_proc = proc_table;
    }
}