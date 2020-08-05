#include <memory.h>

#define MEMMAN_ADDR 0x003c0000 // 内存分配表放置位置

MEMMAN *memman = (MEMMAN *)MEMMAN_ADDR;

u32 mm_init()
{
    memman->frees    = 0; /* 可用信息数目 */
    memman->maxfrees = 0; /* 用于观察可用状况：frees的最大值 */
    memman->lostsize = 0; /* 释放失败的内存的大小总和 */
    memman->losts    = 0; /* 释放失败次数 */

    u32 memtotal = memtest(0x00400000, 0x00480000);
    mm_free(0x00001000, 0x0009e000); /* 0x00001000 - 0x0009efff */
    mm_free(0x00400000, memtotal - 0x00400000);
    return memtotal;
}

u32 mm_total()
{
    u32 i, t = 0;
    for (i = 0; i < memman->frees; i++)
    {
        t += memman->free[i].size;
    }
    return t;
}

u32 mm_alloc(u32 size)
{
    for (int i = 0; i < memman->frees; i++)
    {
        if (memman->free[i].size >= size)
        {
            /* 找到了足够大的内存 */
            u32 a = memman->free[i].addr;
            memman->free[i].addr += size;
            memman->free[i].size -= size;
            if (memman->free[i].size == 0)
            {
                /* 如果free[i]变成了0，就减掉一条可用信息 */
                memman->frees--;
                for (; i < memman->frees; i++)
                {
                    memman->free[i] = memman->free[i + 1]; /* 代入结构体 */
                }
            }
            return a;
        }
    }
    return 0; /* 没有可用空间 */
}

int mm_free(u32 addr, u32 size)
{
    int i, j;
    /* 为便于归纳内存，将free[]按照addr的顺序排列 */
    /* 所以，先决定应该放在哪里 */
    for (i = 0; i < memman->frees; i++)
    {
        if (memman->free[i].addr > addr)
        {
            break;
        }
    }
    /* free[i - 1].addr < addr < free[i].addr */
    if (i > 0)
    {
        /* 前面有可用内存 */
        if (memman->free[i - 1].addr + memman->free[i - 1].size == addr)
        {
            /* 可以与前面的可用内存归纳到一起 */
            memman->free[i - 1].size += size;
            if (i < memman->frees)
            {
                /* 后面也有 */
                if (addr + size == memman->free[i].addr)
                {
                    /* 也可以与后面的可用内存归纳到一起 */
                    memman->free[i - 1].size += memman->free[i].size;
                    /* memman->free[i]删除 */
                    /* free[i]变成0后归纳到前面去 */
                    memman->frees--;
                    for (; i < memman->frees; i++)
                    {
                        memman->free[i] = memman->free[i + 1]; /* 结构体赋值 */
                    }
                }
            }
            return 0; /* 成功完成 */
        }
    }
    /* 不能与前面的可用空间归纳到一起 */
    if (i < memman->frees)
    {
        /* 后面还有 */
        if (addr + size == memman->free[i].addr)
        {
            /* 可以与后面的内容归纳到一起 */
            memman->free[i].addr = addr;
            memman->free[i].size += size;
            return 0; /* 成功完成 */
        }
    }
    /* 既不能与前面归纳到一起，也不能与后面归纳到一起 */
    if (memman->frees < MEMMAN_FREES)
    {
        /* free[i]之后的，向后移动，腾出一点可用空间 */
        for (j = memman->frees; j > i; j--)
        {
            memman->free[j] = memman->free[j - 1];
        }
        memman->frees++;
        if (memman->maxfrees < memman->frees)
        {
            memman->maxfrees = memman->frees; /* 更新最大值 */
        }
        memman->free[i].addr = addr;
        memman->free[i].size = size;
        return 0; /* 成功完成 */
    }
    /* 不能往后移动 */
    memman->losts++;
    memman->lostsize += size;
    return -1; /* 失败 */
}

u32 mm_alloc_4k(u32 size)
{
    u32 a;
    size = (size + 0xfff) & 0xfffff000;
    return mm_alloc(size);
}

int mm_free_4k(u32 addr, u32 size)
{
    int i;
    size = (size + 0xfff) & 0xfffff000;
    i    = mm_free(addr, size);
    return i;
}