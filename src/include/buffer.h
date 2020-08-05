#pragma once
#include <types.h>

#define FIFO_OVERFLOW 0x1

// 循环队列缓冲区,
typedef struct s_fifo_buffer
{
    u8 *buf;
    u8 *p_head;
    u8 *p_tail;
    int capacity, size, flags;
} FIFO_BUFFER;

void fifo_init(FIFO_BUFFER *fifo, int size, u8 *buf);
int fifo_push(FIFO_BUFFER *fifo, u8 data);
int fifo_pop(FIFO_BUFFER *fifo);