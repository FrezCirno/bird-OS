#pragma once

#define EFLAG_CF    (1 << 0)  // Carry Flag
#define EFLAG_RES   (1 << 1)  // reserved, 1
#define EFLAG_PF    (1 << 2)  // Parity Flag
#define EFLAG_AF    (1 << 4)  // Auxiliary Carry Flag
#define EFLAG_ZF    (1 << 6)  // Zero Flag
#define EFLAG_SF    (1 << 7)  // Sign Flag
#define EFLAG_TF    (1 << 8)  // Trap Flag
#define EFLAG_IF    (1 << 9)  // Interrupt Enable Flag
#define EFLAG_DF    (1 << 10) // Direction Flag
#define EFLAG_OF    (1 << 11) // Overflow Flag
#define EFLAG_IOPL  (3 << 12) // I/O Privilege Level
#define EFLAG_IOPL1 (1 << 12) // I/O Privilege Level
#define EFLAG_IOPL2 (1 << 13) // I/O Privilege Level
#define EFLAG_NT    (1 << 14) // Nested Task
#define EFLAG_RF    (1 << 16) // Resume Flag
#define EFLAG_VM    (1 << 17) // Virtual-8086 Mode
#define EFLAG_AC    (1 << 18) // Alignment Check / Access Control
#define EFLAG_VIF   (1 << 19) // Virtual Interrupt Flag
#define EFLAG_VIP   (1 << 20) // Virtual Interrupt Pending
#define EFLAG_ID    (1 << 21) // ID Flag

/* 段描述符, 放在GDT和LDT里 */
/* 或者TSS描述符, 放在GDT里 */
typedef struct s_descriptor /* 共 8 个字节 */
{
    unsigned short limit_low;       /* Limit */
    unsigned short base_low;        /* Base */
    unsigned char base_mid;         /* Base */
    unsigned char attr1;            /* P(1) DPL(2) DT(1) TYPE(4) */
    unsigned char limit_high_attr2; /* G(1) D(1) 0(1) AVL(1) LimitHigh(4) */
    unsigned char base_high;        /* Base */
} DESCRIPTOR;

/* 门描述符, 放在IDT里 */
typedef struct s_gate
{
    unsigned short offset_low; /* Offset Low */
    unsigned short selector;   /* Selector */
    unsigned char dcount; /* 该字段只在调用门描述符中有效。如果在利用
              调用门调用子程序时引起特权级的转换和堆栈
              的改变，需要将外层堆栈中的参数复制到内层
              堆栈。该双字计数字段就是用于说明这种情况
              发生时，要复制的双字参数的数量。*/
    unsigned char attr;         /* P(1) DPL(2) DT(1,=0) TYPE(4) */
    unsigned short offset_high; /* Offset High */
} GATE;

/* 描述符和门的公共属性 */
#define DA_S    0x10 // 描述符类型, 1表示数据段/代码段描述符, 0表示系统段/门描述符
#define DA_P    0x80 // 对数据段:该段是否在内存中; 对门: 被调用时会被处理器置1
#define DA_DPL0 0x00 // DPL = 0
#define DA_DPL1 0x20 // DPL = 1
#define DA_DPL2 0x40 // DPL = 2
#define DA_DPL3 0x60 // DPL = 3

// 代码段和数据段(S=1)的属性字段
#define DA_G DA_S | 0x8000 // 段界限粒度, 1表示4K字节, 0表示单字节
/**
 * DA_D:
 * (兼容性设置)默认的操作数/栈指针大小,
 * 1表示32位保护模式: 32位操作数/使用ESP寄存器,
 * 0表示16位保护模式: 16位操作数/使用SP寄存器.
 * 16位保护模式已经罕见, 所以一般置1
 */
#define DA_D DA_S | 0x4000
#define DA_L DA_S | 0x2000 // 64位代码段标志, 此处置0即可
#define DA_A 0x1 // 最近是否访问过该段, 由处理器自动设置, 需要OS清零

#define DA_DRO  DA_S | 0x0 // 数据段 只读
#define DA_DRW  DA_S | 0x2 // 数据段 读写
#define DA_DESC DA_S | 0x4 // 向下拓展(类似栈)

#define DA_CXO  DA_S | 0x8 // 代码段 只执行
#define DA_CXR  DA_S | 0xA // 代码段 执行,读
#define DA_CONF DA_S | 0x4 // 一致/依从

// 系统段和门描述符(S=0)的属性字段
#define DA_LDT      0x2 // 局部描述符表段
#define DA_TaskGate 0x5 // 任务门
#define DA_386TSS   0x9 // TSS
#define DA_386CGate 0xC // 调用门
#define DA_386IGate 0xE // 中断门
#define DA_386TGate 0xF // 陷阱门

#define BASE_OF_DESC(desc) \
    (((desc).base_high << 24) | ((desc).base_mid << 16) | ((desc).base_low))

/* 描述符表指针, lgdt, lidt指令参数 */
typedef struct s_ptr
{
    unsigned short limit;
    unsigned int base;
} __attribute__((packed)) GIDTPTR;
// packed - 防止结构体自动按n字节对齐, 否则描述符指针结构体会出问题

/* 16位, 段选择子, call, jmp指令参数 */
// 一定是8的倍数, 所以低3位用不到
// 低3位用于描述其他内容
typedef unsigned short SELECTOR;

// 选择子属性
#define SA_TI   0x4 // 1表示使用LDT, 0表示使用GDT
#define SA_RPL0 0x0 // RPL = 0
#define SA_RPL1 0x1 // RPL = 1
#define SA_RPL2 0x2 // RPL = 2
#define SA_RPL3 0x3 // RPL = 3

#define RPL_KRNL 0
#define RPL_TASK 1
#define RPL_USER 3

/* GDT表里已有的条目 */
/* 索引 */
#define INDEX_DUMMY     0 // ┓
#define INDEX_FLAT_C    1 // ┣ BOOT 里面已经确定了的.
#define INDEX_FLAT_RW   2 // ┃
#define INDEX_VIDEO     3 // ┛
#define INDEX_TSS       4
#define INDEX_LDT_FIRST 5

/* 选择子 */
#define SELECTOR_DUMMY     0
#define SELECTOR_FLAT_C    0x08
#define SELECTOR_FLAT_RW   0x10
#define SELECTOR_VIDEO     (0x18 | SA_RPL3)
#define SELECTOR_TSS       0x20 // TSS. 从外层跳到内层时 SS 和 ESP 的值从里面获得.
#define SELECTOR_LDT_FIRST 0x28

#define SELECTOR_KERNEL_CS SELECTOR_FLAT_C
#define SELECTOR_KERNEL_DS SELECTOR_FLAT_RW
#define SELECTOR_KERNEL_GS SELECTOR_VIDEO

/* 描述符的个数 */
#define GDT_SIZE 128
#define IDT_SIZE 256
#define LDT_SIZE 2

/* 进入内核之后的gdt和idt, 定义位于 protect.c 中 */
extern DESCRIPTOR gdt[GDT_SIZE];

extern GIDTPTR gdt_ptr; // 0~15:Limit  16~47:Base

extern GATE idt[IDT_SIZE];

extern GIDTPTR idt_ptr; // 0~15:Limit  16~47:Base

void set_seg_desc(DESCRIPTOR *pDesc, unsigned int base, unsigned int limit,
                  unsigned short attribute);

// 段基地址(绝对) + 虚拟地址 -> 物理地址
#define vir2phys(seg_base, vir) \
    (unsigned int)((unsigned int)(seg_base) + (unsigned int)(vir))

// gdt的选择子 -> 该段的基地址
unsigned int gdt_desc_base(SELECTOR seg);