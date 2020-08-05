#include <int.h>    //
#include <glib.h>   // drawText
#include <string.h> // itoa

irq_handler irq_table[NR_IRQ] = { // 0x1010af
    default_irq_handler, default_irq_handler, default_irq_handler,
    default_irq_handler, default_irq_handler, default_irq_handler,
    default_irq_handler, default_irq_handler, default_irq_handler,
    default_irq_handler, default_irq_handler, default_irq_handler,
    default_irq_handler, default_irq_handler, default_irq_handler,
    default_irq_handler};

void put_irq_handler(u32 irq, int_handler handler)
{
    disable_irq(irq);
    irq_table[irq] = handler;
}

void default_irq_handler(u32 irq)
{
    printstr("IRQ: ", PEN_WHITE);
    printstr(itoa(irq, 16), PEN_WHITE);
}
