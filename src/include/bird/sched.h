#pragma once

// 这里定义了进程运行时可能处的状态。

// 进程正在运行或已准备就绪。
#define TASK_RUNNING 0
// 进程处于可中断等待状态。
#define TASK_INTERRUPTIBLE 1
// 进程处于不可中断等待状态，主要用于 I/O 操作等待。
#define TASK_UNINTERRUPTIBLE 2
// 进程处于僵死状态，已经停止运行，但父进程还没发信号。
#define TASK_ZOMBIE 3
// 进程已停止。
#define TASK_STOPPED 4
