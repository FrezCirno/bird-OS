// 系统调用表 int 0x80会call这里
#include <bird/sys.h> // syscall_table
#include <clock.h>    // sys_getticks

// 这里要和各处文件内定义的系统调用一致
void *syscall_table[NR_SYS_CALL] = {sys_nothing, sys_getticks};

_syscall0(int, nothing);

int sys_nothing()
{
    return 0;
}