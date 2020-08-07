#pragma once

void memcpy(void *dst, const void *src, int size);

void memset(void *buf, unsigned char val, unsigned int size);

void strcpy(unsigned char *dst, const unsigned char *src);

const char *itoa(int x, int radix);