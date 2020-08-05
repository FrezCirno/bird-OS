#include <int.h> // syscall_table
#include <clock.h> // ticks

// 系统调用表
void *syscall_table[NR_SYS_CALL] = {
    sys_get_ticks, sys_nothing, sys_nothing, sys_nothing, sys_nothing,
    sys_nothing,   sys_nothing, sys_nothing, sys_nothing, sys_nothing};

int sys_get_ticks()
{
    return ticks;
}

int sys_nothing()
{
    return 0;
}