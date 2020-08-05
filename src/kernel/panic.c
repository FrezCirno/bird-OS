#include <glib.h>

void panic(const char *str)
{
    printstr("Kernel Panic!", PEN_RED);
    for (;;)
        ;
}