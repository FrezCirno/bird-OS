#include <bird/bird.h>
#include <glib.h>
#include <printf.h>

#define MAG_CH_PANIC  '\002'
#define MAG_CH_ASSERT '\003'

void assertion_failure(char *exp, char *file, char *base_file, int line)
{
    gotoxy(0, 0);
    printstr("assertion_failure", PEN_WHITE);
    // printl("%c  assert(%s) failed: file: %s, base_file: %s, ln%d",
    //        MAG_CH_ASSERT, exp, file, base_file, line);

    /**
     * If assertion fails in a TASK, the system will halt before
     * printl() returns. If it happens in a USER PROC, printl() will
     * return like a common routine and arrive here.
     * @see sys_printx()
     *
     * We use a forever loop to prevent the proc from going on:
     */
    panic("assertion_failure()");

    /* should never arrive here */
    __asm__ __volatile__("ud2");
}