#pragma once

#define TTY_IN_BYTES 256 /* tty input queue size */

/* CONSOLE */
typedef struct s_console
{
    unsigned char current_start_addr; /* 当前显示到了什么位置	  */
    unsigned char original_addr;      /* 当前控制台对应显存位置 */
    unsigned char v_mem_limit;        /* 当前控制台占的显存大小 */
    unsigned char cursor;             /* 当前光标位置 */
} CONSOLE;

/* TTY */
typedef struct s_tty
{
    unsigned int in_buf[TTY_IN_BYTES]; /* TTY 输入缓冲区 */
    unsigned int *p_inbuf_head; /* 指向缓冲区中下一个空闲位置 */
    unsigned int *p_inbuf_tail; /* 指向键盘任务应处理的键值 */
    int inbuf_count;            /* 缓冲区中已经填充了多少 */

    CONSOLE *p_console;
} TTY;
