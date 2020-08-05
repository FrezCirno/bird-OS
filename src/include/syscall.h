// 用户程序系统调用头文件
#pragma once

#define SYS_GET_TICKS 0x0

int syscall(u32 id);
