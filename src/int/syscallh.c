#include <int.h>   // syscall_table
#include <clock.h> // ticks

// 系统调用表
void *syscall_table[NR_SYS_CALL] = {
    sys_getticks, sys_nothing, sys_nothing, sys_nothing, sys_nothing,
    sys_nothing,  sys_nothing, sys_nothing, sys_nothing, sys_nothing};

int sys_nothing()
{
    return 0;
}