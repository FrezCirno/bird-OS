#include <asm/io.h>
#include <glib.h>   // drawText
#include <string.h> // itoa
#include <int.h>    // NR_IRQ

irq_handler irq_table[NR_IRQ] = {
    default_irq_handler, default_irq_handler, default_irq_handler,
    default_irq_handler, default_irq_handler, default_irq_handler,
    default_irq_handler, default_irq_handler, default_irq_handler,
    default_irq_handler, default_irq_handler, default_irq_handler,
    default_irq_handler, default_irq_handler, default_irq_handler,
    default_irq_handler};

// 初始化8259A
void init_pic()
{
    out8(INT_PORT_MASTER_CMD, 0x11); // Master 8259, ICW1.
    out8(INT_PORT_SLAVE_CMD, 0x11);  // Slave  8259, ICW1.

    /* Master 8259, ICW2. 设置 '主8259' 的中断入口地址为 0x20. */
    out8(INT_PORT_MASTER_DATA, INT_VECTOR_IRQ);
    /* Slave  8259, ICW2. 设置 '从8259' 的中断入口地址为 0x28 */
    out8(INT_PORT_SLAVE_DATA, INT_VECTOR_IRQ + 8);

    out8(INT_PORT_MASTER_DATA, 0x4); // Master 8259, ICW3. IR2 对应 '从8259'.
    out8(INT_PORT_SLAVE_DATA, 0x2); // Slave  8259, ICW3. 对应 '主8259' 的 IR2.

    out8(INT_PORT_MASTER_DATA, 0x1); // Master 8259, ICW4.
    out8(INT_PORT_SLAVE_DATA, 0x1);  // Slave  8259, ICW4.

    // 只保留Master的2号中断
    out8(INT_PORT_MASTER_DATA, 0xFB); // Master 8259, OCW1.
    out8(INT_PORT_SLAVE_DATA, 0xFF);  // Slave  8259, OCW1.
}

void put_irq_handler(unsigned int irq, irq_handler handler)
{
    disable_irq(irq);
    irq_table[irq] = handler;
}

void default_irq_handler(unsigned int irq)
{
    printstr("IRQ: ", PEN_WHITE);
    printstr(itoa(irq, 10), PEN_WHITE);
}

void disable_irq(unsigned int irq)
{
    unsigned int e = load_eflags();
    cli();
    if (irq < 8)
        out8(INT_PORT_MASTER_DATA, in8(INT_PORT_MASTER_DATA) | (1 << irq));
    else
        out8(INT_PORT_SLAVE_DATA, in8(INT_PORT_SLAVE_DATA) | (1 << (irq - 8)));
    store_eflags(e);
}

void enable_irq(unsigned int irq)
{
    unsigned int e = load_eflags();
    cli();
    if (irq < 8)
        out8(INT_PORT_MASTER_DATA, in8(INT_PORT_MASTER_DATA) & ~(1 << irq));
    else
        out8(INT_PORT_SLAVE_DATA, in8(INT_PORT_SLAVE_DATA) & ~(1 << (irq - 8)));
    store_eflags(e);
}