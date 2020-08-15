#pragma once
#include <bird/proc.h>
#include <ata.h>

struct hd_info
{
    int base;    //起始扇区LBA
    int nsector; //扇区数目
};

extern unsigned char hd_num;
extern struct hd_info hd[];

typedef struct s_hd_request
{
    struct ata_cmd cmd;        // 存储需要执行的指令
    unsigned char *buf;        // 读/写的缓冲区
    int errors;                // 错误次数
    int lock;                  // 是否锁定
    PROCESS *proc;             // 发出请求的进程
    struct s_hd_request *next; // 静态链表
} HD_REQUEST;

// 初始化硬盘驱动程序
void init_hd();

// 硬盘中断处理程序
void hd_handler(unsigned int irq);

// 向硬盘发送重置信号, 完成后继续 hd_process
void hd_reset(int drive);

// 向硬盘驱动器发送控制命令, 并设置回调函数
void hd_sendcmd(struct ata_cmd *cmd, void (*int_callback)(void));

// 检查cmd是否成功执行
int hd_cmd_ok();

// 等待硬盘驱动器的某个状态位被设置, 等到返回1, 否则返回0
int hd_wait(int mask, int val, int timeoutms);

// 是否busy, 发送cmd之前应该检查
int hd_busy();

// 向请求表中添加一个请求
void add_request(HD_REQUEST *req);

// 处理当前硬盘请求(this_request)/继续处理下一个扇区
void hd_process();

// 完成一个request, 切换到下一个的处理函数
void hd_request_fin();

// 硬盘处理完毕发出中断, 中断的处理函数
// 与sendcmd配套
void bad_rw_intr();
void read_intr();
void write_intr();

void hd_rw(int rw, unsigned char drive, unsigned int lba, unsigned char nsector,
           unsigned char *buf);

void print_identify_info(unsigned short *hdinfo);
void hd_identify(int drive);
void wait_request(HD_REQUEST *req);
void unlock_request(HD_REQUEST *req);
