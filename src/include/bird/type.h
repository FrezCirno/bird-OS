#pragma once

typedef unsigned char u8;
typedef signed char s8;
typedef unsigned short u16;
typedef short s16;
typedef unsigned int u32;
typedef int s32;

typedef void (*int_handler)();        // 异常处理程序
typedef void (*irq_handler)(u32 irq); // 中断处理程序