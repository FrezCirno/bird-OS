#include <asm/io.h>    // out8
#include <string.h>    // memcpy
#include <protect.h>   // GATE, idt, gdt, tss
#include <int.h>       // INT_VECTOR_*
#include <bird/proc.h> // PROCESS

/* 8259A interrupt controller ports. */
#define INT_PORT_MASTER_CMD  0x20
#define INT_PORT_MASTER_DATA 0x21
#define INT_PORT_SLAVE_CMD   0xA0
#define INT_PORT_SLAVE_DATA  0xA1

/* 中断/异常处理函数, 定义都是汇编代码, 位于 int_entry.asm 中 */
void divide_error();
void single_step_exception();
void nmi();
void breakpoint_exception();
void overflow();
void bounds_check();
void inval_opcode();
void copr_not_available();
void double_fault();
void copr_seg_overrun();
void inval_tss();
void segment_not_present();
void stack_exception();
void general_protection();
void page_fault();
void copr_error();
void hwint00();
void hwint01();
void hwint02();
void hwint03();
void hwint04();
void hwint05();
void hwint06();
void hwint07();
void hwint08();
void hwint09();
void hwint10();
void hwint11();
void hwint12();
void hwint13();
void hwint14();
void hwint15();
void syscall_entry();

// 将 boot 中的 GDT 搬到内核区
void init_gdt_idt()
{
    /* 将 LOADER 中的 GDT 复制到新的 GDT 中 */
    memcpy((void *)&gdt,         /* New GDT */
           (void *)gdt_ptr.base, /* Base of Old GDT */
           gdt_ptr.limit + 1);   /* Limit of Old GDT */

    /* gdt_ptr[6] 共 6 个字节：0~15:Limit  16~47:Base。用作 sgdt/lgdt 的参数。*/
    gdt_ptr.limit = GDT_SIZE * sizeof(DESCRIPTOR) - 1;
    gdt_ptr.base  = (unsigned int)&gdt;

    /* idt_ptr[6] 共 6 个字节：0~15:Limit  16~47:Base。用作 sidt/lidt 的参数。*/
    idt_ptr.limit = IDT_SIZE * sizeof(GATE) - 1;
    idt_ptr.base  = (unsigned int)&idt;
}

// 初始化 386 中断门
void init_igate(unsigned char vector, unsigned char desc_type,
                int_handler handler, unsigned char privilege)
{
    GATE *pGate        = &idt[vector];
    unsigned int base  = (unsigned int)handler;
    pGate->offset_low  = base & 0xFFFF;
    pGate->selector    = SELECTOR_KERNEL_CS;
    pGate->dcount      = 0;
    pGate->attr        = DA_P | desc_type | (privilege << 5);
    pGate->offset_high = (base >> 16) & 0xFFFF;
}

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

// 初始化 IDT
void setup_idt()
{
    // 全部初始化成中断门(没有陷阱门)
    // 硬件异常 0-19
    init_igate(INT_VECTOR_DIVIDE, DA_386IGate, divide_error, RPL_KRNL);
    init_igate(INT_VECTOR_DEBUG, DA_386IGate, single_step_exception, RPL_KRNL);
    init_igate(INT_VECTOR_NMI, DA_386IGate, nmi, RPL_KRNL);
    init_igate(INT_VECTOR_BREAKPOINT, DA_386IGate, breakpoint_exception,
               RPL_USER);
    init_igate(INT_VECTOR_OVERFLOW, DA_386IGate, overflow, RPL_USER);
    init_igate(INT_VECTOR_BOUNDS, DA_386IGate, bounds_check, RPL_KRNL);
    init_igate(INT_VECTOR_INVAL_OP, DA_386IGate, inval_opcode, RPL_KRNL);
    init_igate(INT_VECTOR_COPROC_NOT, DA_386IGate, copr_not_available, RPL_KRNL);
    init_igate(INT_VECTOR_DOUBLE_FAULT, DA_386IGate, double_fault, RPL_KRNL);
    init_igate(INT_VECTOR_COPROC_SEG, DA_386IGate, copr_seg_overrun, RPL_KRNL);
    init_igate(INT_VECTOR_INVAL_TSS, DA_386IGate, inval_tss, RPL_KRNL);
    init_igate(INT_VECTOR_SEG_NOT, DA_386IGate, segment_not_present, RPL_KRNL);
    init_igate(INT_VECTOR_STACK_FAULT, DA_386IGate, stack_exception, RPL_KRNL);
    init_igate(INT_VECTOR_PROTECTION, DA_386IGate, general_protection, RPL_KRNL);
    init_igate(INT_VECTOR_PAGE_FAULT, DA_386IGate, page_fault, RPL_KRNL);
    init_igate(INT_VECTOR_COPROC_ERR, DA_386IGate, copr_error, RPL_KRNL);
    // 中断 32-255
    init_igate(INT_VECTOR_IRQ + 0, DA_386IGate, hwint00, RPL_KRNL);
    init_igate(INT_VECTOR_IRQ + 1, DA_386IGate, hwint01, RPL_KRNL);
    init_igate(INT_VECTOR_IRQ + 2, DA_386IGate, hwint02, RPL_KRNL);
    init_igate(INT_VECTOR_IRQ + 3, DA_386IGate, hwint03, RPL_KRNL);
    init_igate(INT_VECTOR_IRQ + 4, DA_386IGate, hwint04, RPL_KRNL);
    init_igate(INT_VECTOR_IRQ + 5, DA_386IGate, hwint05, RPL_KRNL);
    init_igate(INT_VECTOR_IRQ + 6, DA_386IGate, hwint06, RPL_KRNL);
    init_igate(INT_VECTOR_IRQ + 7, DA_386IGate, hwint07, RPL_KRNL);
    init_igate(INT_VECTOR_IRQ + 8, DA_386IGate, hwint08, RPL_KRNL);
    init_igate(INT_VECTOR_IRQ + 9, DA_386IGate, hwint09, RPL_KRNL);
    init_igate(INT_VECTOR_IRQ + 10, DA_386IGate, hwint10, RPL_KRNL);
    init_igate(INT_VECTOR_IRQ + 11, DA_386IGate, hwint11, RPL_KRNL);
    init_igate(INT_VECTOR_IRQ + 12, DA_386IGate, hwint12, RPL_KRNL);
    init_igate(INT_VECTOR_IRQ + 13, DA_386IGate, hwint13, RPL_KRNL);
    init_igate(INT_VECTOR_IRQ + 14, DA_386IGate, hwint14, RPL_KRNL);
    init_igate(INT_VECTOR_IRQ + 15, DA_386IGate, hwint15, RPL_KRNL);
    // 0x90 系统中断
    init_igate(INT_VECTOR_SYS_CALL, DA_386IGate, syscall_entry, RPL_USER);
}

// 初始化 GDT 中的 LDT 描述符, 有几个任务就需要几个 LDT 描述符
// LDT 描述符从SELECTOR_LDT_FIRST开始
// void init_proc_ldts()
// {
//     PROCESS *pProc   = proc_table;
//     SELECTOR ldt_slt = SELECTOR_LDT_FIRST;
//     for (int i = 0; i < NR_TASKS; i++)
//     {
//         set_seg_desc(&gdt[INDEX_LDT_FIRST],
//                  vir2phys(seg2phys(SELECTOR_KERNEL_DS), pProc->ldt),
//                  LDT_SIZE * sizeof(DESCRIPTOR) - 1, DA_P | DA_LDT);
//         pProc++;
//         ldt_slt += 0x8;
//     }
// }

// 初始化 GDT 中的 TSS 描述符和 TSS
void init_tss()
{
    memset(&tss, 0, sizeof(tss));
    tss.ss0    = SELECTOR_KERNEL_DS;
    tss.iobase = sizeof(tss); /* 没有I/O许可位图 */
    set_seg_desc(&gdt[INDEX_TSS], vir2phys(seg2phys(SELECTOR_KERNEL_DS), &tss),
                 sizeof(tss) - 1, DA_P | DA_386TSS);
}
