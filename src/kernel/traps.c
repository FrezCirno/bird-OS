#include <glib.h>   // printstr
#include <string.h> // itoa

// <R0> 异常处理函数
void exception_handler(unsigned int vec_no, unsigned int err_code,
                       unsigned int eip, unsigned int cs, unsigned int eflags)
{
    static const char *err_msg[] = {
        "#DE Divide Error",
        "#DB RESERVED",
        "—  NMI Interrupt",
        "#BP Breakpoint",
        "#OF Overflow",
        "#BR BOUND Range Exceeded",
        "#UD Invalid Opcode (Undefined Opcode)",
        "#NM Device Not Available (No Math Coprocessor)",
        "#DF Double Fault",
        "    Coprocessor Segment Overrun (reserved)",
        "#TS Invalid TSS",
        "#NP Segment Not Present",
        "#SS Stack-Segment Fault",
        "#GP General Protection",
        "#PF Page Fault",
        "—  (Intel reserved. Do not use.)",
        "#MF x87 FPU Floating-Point Error (Math Fault)",
        "#AC Alignment Check",
        "#MC Machine Check",
        "#XF SIMD Floating-Point Exception"};

    int text_color = PEN_WHITE;

    printstr("Exception! --> ", text_color);
    printstr(err_msg[vec_no], text_color);

    printstr("\nEFLAGS: 0x", text_color);
    printstr(itoa(eflags, 16), text_color);

    printstr("\nCS: 0x", text_color);
    printstr(itoa(cs, 16), text_color);

    printstr("\nEIP: 0x", text_color);
    printstr(itoa(eip, 16), text_color);

    if (err_code != 0xFFFFFFFF)
    {
        printstr("\nError code: 0x", text_color);
        printstr(itoa(err_code, 16), text_color);
    }
}
