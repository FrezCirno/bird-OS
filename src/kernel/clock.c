#include <asm/io.h>
#include <bird/proc.h> // next_proc
#include <int.h>       // put_irq_handler
#include <clock.h>     // declarations

#define PIT_PORT_CTRL 0x43
#define PIT_PORT_CNT0 0x40

int ticks;

void init_clock()
{
    out8(PIT_PORT_CTRL, 0x34);
    out8(PIT_PORT_CNT0, 0x9c);
    out8(PIT_PORT_CNT0, 0x2e);

    ticks = 0;

    put_irq_handler(INT_VECTOR_IRQ_CLOCK, clock_handler);
    enable_irq(INT_VECTOR_IRQ_CLOCK);
}

// 时钟中断处理器, 主要任务是计时和调度程序
void clock_handler(unsigned int irq)
{
    ticks++;
    next_proc->ticks--;
    if (k_reenter != 0) return;
    schedule();
}

int sys_getticks()
{
    return ticks;
}
