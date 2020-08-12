#include <asm/io.h>
#include <bird/proc.h> // next_proc
#include <bird/sys.h>  // _syscall
#include <int.h>       // put_irq_handler
#include <clock.h>     // declarations
#include <string.h>
#include <stddef.h>

TIMERCTL timerctl;

void init_clock()
{
    out8(PIT_PORT_CTRL, RATE_GENERATOR);
    out8(PIT_PORT_CNT0, 0x9c);
    out8(PIT_PORT_CNT0, 0x2e);

    timerctl.ticks   = 0;
    timerctl.closest = 0xffffffff;
    memset((void *)timerctl._timers, 0, sizeof(timerctl._timers));

    put_irq_handler(INT_VECTOR_IRQ_CLOCK, clock_handler);
    enable_irq(INT_VECTOR_IRQ_CLOCK);
}

// 时钟中断处理器, 主要任务是计时和调度程序
void clock_handler(unsigned int irq)
{
    int i;
    timerctl.ticks++;
    current->ticks--;

    if (timerctl.closest <= timerctl.ticks)
    {
        timerctl.closest = 0xffffffff;
        for (i = 0; i < timerctl.count; i++)
        {
            if (timerctl.timers[i]->timeout > timerctl.ticks) break;
            timerctl.timers[i]->flags = TIMER_FLAGS_ALLOC;
            fifo_push(timerctl.timers[i]->buffer, timerctl.timers[i]->data);
        }
        timerctl.count -= i;
        for (int j = 0; j < timerctl.count; j++)
        {
            timerctl.timers[j] = timerctl.timers[i + j];
        }
        if (timerctl.count > 0)
        {
            timerctl.closest = timerctl.timers[0]->timeout;
        }
        else
        {
            timerctl.closest = 0xffffffff;
        }
    }
    if (k_reenter != 0) return;
    schedule();
}

int sys_getticks()
{
    return timerctl.ticks;
}

_syscall0(int, getticks);

TIMER *alloc_timer()
{
    for (int i = 0; i < MAX_TIMER_COUNT; i++)
    {
        TIMER *timer = timerctl.timers[i];
        if (timer->flags == 0)
        {
            timer->flags = TIMER_FLAGS_ALLOC;
            return timer;
        }
    }
    return NULL;
}

void free_timer(TIMER *timer)
{
    timer->flags = 0;
}

void timer_init(TIMER *timer, FIFO_BUFFER *buffer, unsigned char data)
{
    timer->buffer = buffer;
    timer->data   = data;
}

void timer_settime(TIMER *timer, unsigned int timeout)
{
    int i;
    timer->timeout = timeout + timerctl.count;
    timer->flags   = TIMER_FLAGS_ACTIVE;
    int eflags     = load_eflags();
    cli();
    /* 搜索注册位置 */
    for (i = 0; i < timerctl.count; i++)
    {
        if (timerctl.timers[i]->timeout >= timer->timeout)
        {
            break;
        }
    }
    /* i号之后全部后移一位 */
    for (int j = timerctl.count; j > i; j--)
    {
        timerctl.timers[j] = timerctl.timers[j - 1];
    }
    timerctl.count++;
    /* 插入到空位上 */
    timerctl.timers[i] = timer;
    timerctl.closest   = timerctl.timers[0]->timeout;
    store_eflags(eflags);
}