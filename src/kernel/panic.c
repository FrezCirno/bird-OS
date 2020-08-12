#include <glib.h>

void panic(const char *str)
{
    printstr("Kernel Panic!", PEN_RED);
    printstr(str, PEN_RED);
    __asm__("cli;hlt;1:jmpl 1");
}