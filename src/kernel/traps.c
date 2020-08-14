#include <bird/bird.h> // printstr

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

    printk("Exception! --> %s\n", text_color, err_msg[vec_no]);
    printk("EFLAGS: 0x%x\n", text_color, eflags);
    printk("CS: 0x%x\n", text_color, cs);
    printk("EIP: 0x%x\n", text_color, eip);

    if (err_code != 0xFFFFFFFF)
    {
        printk("Error code: 0x%x\n", text_color, err_code);
    }
}
