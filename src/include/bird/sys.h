// 这里来自linux
// 用户程序进行系统调用需要的头文件
// 所有系统调用的声明都放在这里

#pragma once
#include <stddef.h>

#define NR_SYS_CALL 10
extern void *syscall_table[NR_SYS_CALL];
int sys_nothing();

#define __NR_nothing  0
#define __NR_getticks 1

int getticks();
int nothing();

#define _syscall0(type, name)                                           \
    type name(void)                                                     \
    {                                                                   \
        type __res;                                                     \
        __asm__ volatile("int $0x80" : "=a"(__res) : "0"(__NR_##name)); \
        if (__res >= 0) return __res;                                   \
        errno = -__res;                                                 \
        return -1;                                                      \
    }

#define _syscall1(type, name, atype, a)               \
    type name(atype a)                                \
    {                                                 \
        type __res;                                   \
        __asm__ volatile("int $0x80"                  \
                         : "=a"(__res)                \
                         : "0"(__NR_##name), "b"(a)); \
        if (__res >= 0) return __res;                 \
        errno = -__res;                               \
        return -1;                                    \
    }

#define _syscall2(type, name, atype, a, btype, b)             \
    type name(atype a, btype b)                               \
    {                                                         \
        type __res;                                           \
        __asm__ volatile("int $0x80"                          \
                         : "=a"(__res)                        \
                         : "0"(__NR_##name), "b"(a), "c"(b)); \
        if (__res >= 0) return __res;                         \
        errno = -__res;                                       \
        return -1;                                            \
    }

#define _syscall3(type, name, atype, a, btype, b, ctype, c)           \
    type name(atype a, btype b, ctype c)                              \
    {                                                                 \
        type __res;                                                   \
        __asm__ volatile("int $0x80"                                  \
                         : "=a"(__res)                                \
                         : "0"(__NR_##name), "b"(a), "c"(b), "d"(c)); \
        if (__res < 0) errno = -__res, __res = -1;                    \
        return __res;                                                 \
    }

extern int errno;