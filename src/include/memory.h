#pragma once
#include <types.h>

// memtest.c
#define EFLAGS_AC_BIT     0x00040000
#define CR0_CACHE_DISABLE 0x60000000

u32 memtest_sub(u32 start, u32 end);
u32 memtest(u32 start, u32 end);

// memory.c
#define MEMMAN_FREES 4090 /* 大约是32KB*/

typedef struct s_FREEINFO
{ /* 可用信息 */
    u32 addr, size;
} FREEINFO;

typedef struct s_MEMMAN
{ /* 内存管理 */
    int frees, maxfrees, lostsize, losts;
    FREEINFO free[MEMMAN_FREES];
} MEMMAN;

u32 mm_init();
u32 mm_total();
u32 mm_alloc(u32 size);
int mm_free(u32 addr, u32 size);
u32 mm_alloc_4k(u32 size);
int mm_free_4k(u32 addr, u32 size);
