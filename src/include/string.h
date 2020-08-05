#pragma once
#include <types.h>

void memcpy(void *dst, const void *src, s32 size);

void memset(void *buf, u8 val, u32 size);

void strcpy(u8 *dst, const u8 *src);

const char *itoa(int x, int radix);