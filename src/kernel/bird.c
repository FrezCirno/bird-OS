#include <bird/bird.h>
#include <asm/io.h>
#include <clock.h>
#include <stdarg.h>
#include <string.h>

void panic(const char *str)
{
    printk("Kernel Panic!\n%s\n", PEN_RED, str);
    for (;;);
}

void printk(char *fmt, int color, ...)
{
    static char buf[512];

    sprintf(buf, "[%d]", sys_getticks());
    printstr(buf, color);

    va_list ap;
    va_start(ap, color);
    vsprintf(buf, fmt, ap);
    va_end(ap);
    printstr(buf, color);
}