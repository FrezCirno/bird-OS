#pragma once
#include <buffer.h>

#define MAX_TIMER_COUNT 500

#define PIT_PORT_CTRL 0x43
#define PIT_PORT_CNT0 0x40

#define RATE_GENERATOR 0x34
#define TIMER_FREQ     1193182L /* clock frequency for timer in PC and AT */
#define HZ             100      /* clock freq (software settable on IBM-PC) */

#define TIMER_FLAGS_ALLOC  1
#define TIMER_FLAGS_ACTIVE 2

typedef struct s_timer
{
    unsigned int count;
    unsigned int timeout; // 表示既定时刻
    FIFO_BUFFER *buffer;
    unsigned char data;
    int flags;
} TIMER;

typedef struct s_timerctl
{
    unsigned int ticks;   // 当前ticks
    unsigned int closest; // 表示最近的计时器的既定时间
    unsigned int count;   // 表示当前活动计时器的个数
    TIMER *timers[MAX_TIMER_COUNT];
    TIMER _timers[MAX_TIMER_COUNT];
} TIMERCTL;

void init_clock();

void clock_handler(unsigned int irq);

int sys_getticks();