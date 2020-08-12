#pragma once

#define nop() __asm__("nop")

#define hlt() __asm__("hlt")

#define sti() __asm__("sti")

#define cli() __asm__("cli")

#define pushfl() __asm__("pushfl")

#define popfl() __asm__("popfl")

#define load_eflags()         \
    ({                        \
        unsigned int res;     \
        __asm__("pushfl\n\t"  \
                "popl %0"     \
                : "=m"(res)); \
        res;                  \
    })

#define store_eflags(e)   \
    __asm__("push %0\n\t" \
            "popfl" ::"m"(e))

#define load_cr0()                                \
    ({                                            \
        unsigned int res;                         \
        __asm__("movl %%cr0, %%eax" : "=a"(res)); \
        res;                                      \
    })

#define store_cr0(e) __asm__("movl %0, %%cr0" ::"r"(e))

#define load_cr1()                                \
    ({                                            \
        unsigned int res;                         \
        __asm__("movl %%cr1, %%eax" : "=a"(res)); \
        res;                                      \
    })

#define store_cr1(e) __asm__("movl %0, %%cr1" ::"r"(e))

#define load_cr2()                                \
    ({                                            \
        unsigned int res;                         \
        __asm__("movl %%cr2, %%eax" : "=a"(res)); \
        res;                                      \
    })

#define store_cr2(e) __asm__("movl %0, %%cr2" ::"r"(e))

#define load_cr3()                             \
    ({                                         \
        unsigned int res;                      \
        __asm__("movl %%cr3, %0" : "=m"(res)); \
        res;                                   \
    })

#define store_cr3(e) __asm__("movl %0, %%cr3" ::"r"(e))

#define in8(port)                                                        \
    ({                                                                   \
        unsigned char _v;                                                \
        __asm__("inb %%dx,%%al" : "=a"(_v) : "d"((unsigned short)port)); \
        _v;                                                              \
    })

#define out8(port, val)                                  \
    __asm__("outb %%al, %%dx" ::"a"((unsigned char)val), \
            "d"((unsigned short)port))

#define in16(port)                                                        \
    ({                                                                    \
        unsigned short _v;                                                \
        __asm__("inw %%dx, %%ax" : "=a"(_v) : "d"((unsigned short)port)); \
        _v;                                                               \
    })

#define out16(port, val)                                  \
    __asm__("outw %%ax, %%dx" ::"a"((unsigned short)val), \
            "d"((unsigned short)port))

#define in32(port)                                                         \
    ({                                                                     \
        unsigned int _v;                                                   \
        __asm__("inl %%dx, %%eax" : "=a"(_v) : "d"((unsigned short)port)); \
        _v;                                                                \
    })

#define out32(port, val)                                 \
    __asm__("outl %%eax, %%dx" ::"a"((unsigned int)val), \
            "d"((unsigned short)port))
