#include <protect.h> // DESCRIPTOR, GATE, TSS

DESCRIPTOR gdt[GDT_SIZE];
GIDTPTR gdt_ptr; // 0~15:Limit  16~47:Base

GATE idt[IDT_SIZE];
GIDTPTR idt_ptr; // 0~15:Limit  16~47:Base

TSS tss;

void set_seg_desc(DESCRIPTOR *pDesc, unsigned int base, unsigned int limit,
                  unsigned short attr)
{
    pDesc->limit_low = limit & 0xFFFF;      /* 段界限 1        (2 字节) */
    pDesc->base_low  = base & 0xFFFF;       /* 段基址 1        (2 字节) */
    pDesc->base_mid  = (base >> 16) & 0xFF; /* 段基址 2        (1 字节) */
    pDesc->attr1     = attr & 0xFF;         /* 属性 1 */
    pDesc->limit_high_attr2 =
        ((limit >> 16) & 0xF) | ((attr >> 8) & 0xF0); /* 段界限 2 + 属性 2 */
    pDesc->base_high = (base >> 24) & 0xFF; /* 段基址 3        (1 字节) */
}

unsigned int seg2phys(unsigned short seg)
{
    DESCRIPTOR *pDest = &gdt[seg >> 3];
    return (pDest->base_high << 24) | (pDest->base_mid << 16) | (pDest->base_low);
}
