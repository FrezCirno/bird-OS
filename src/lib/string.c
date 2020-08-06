#include <types.h>

void memcpy(void *dst, const void *src, s32 size)
{
    for (int i = 0; i < size; i++)
    {
        *(u8 *)dst++ = *(u8 *)src++;
    }
}

void memset(void *buf, u8 val, u32 size)
{
    while (size--)
    {
        *(u8 *)buf++ = val;
    }
}

void strcpy(u8 *dst, const u8 *src)
{
    while (*src)
    {
        *dst++ = *src++;
    }
}

const char *itoa(int x, int radix)
{
    static const char digits[16] = "0123456789ABCDEF";
    static char str[34];
    int flag = 0;
    char *p  = str + sizeof(str);

    *--p = '\0';
    if (x < 0)
    {
        flag = 1;
        x    = -x;
    }
    else if (x == 0)
    {
        *--p = '0';
        return p;
    }
    while (x)
    {
        *--p = digits[x % radix];
        x /= radix;
    }
    if (flag)
    {
        *--p = '-';
    }

    return p;
}