#pragma once
#include <bird/proc.h>
#include <ata.h>

#define MAX_IO_BYTES 256 /* how many sectors does one IO can handle */

// linux编号规则
// 主设备号: 表示设备是硬盘/软盘/内存盘
// 次设备号: 用来区别分区
// 每个设备的主分区表直接指明的分区用1-4表示, 可能不全用到, 如sda1, sda2, ...
// 每个拓展分区下细分的逻辑分区用5之后的数字表示, 如sda5, sda6, ...
// 举例1: sda = [sda1, sda2, sda3, [sda5, sda6, sda7]]
// 举例2: sda = [sda1, sda2, [sda5, sda6], [sda7, sda8, sda9]]
// 举例3: sda = [sda1, [sda5, sda6, sda7, sda8, sda9]]

// ATA/IDE接口: >=2个 - Primary和Secondary, 分别有两套IO端口
// 每个接口可以连接两个设备: Master和Slave (由HD_DEVICE_SLAVE标志区分)
#define MAX_DRIVES        2 // 最大设备数(sda, sdb, ...)(这里只考虑Primary)
#define NR_PART_PER_DRIVE 4 // 每个设备的主分区表最大分区数(主分区+拓展分区)
#define NR_SUB_PER_PART   16 // 每个拓展分区的最大逻辑分区数
// 每个设备的最大逻辑分区数(sda1, sda2, ...)
#define NR_SUB_PER_DRIVE (NR_SUB_PER_PART * NR_PART_PER_DRIVE)
// 整个设备和逻辑分区放在一起编号
// 类似 hd0 = [hd1, hd2, hd3, hd4]
#define NR_PRIM_PER_DRIVE (NR_PART_PER_DRIVE + 1)

/**
 * @def MAX_PRIM
 * Defines the max minor number of the primary partitions.
 * If there are 2 disks, prim_dev ranges in hd[0-9], this macro will
 * equals 9.
 */
#define MAX_PRIM (MAX_DRIVES * NR_PRIM_PER_DRIVE - 1) // 最大主分区数量

#define MINOR_hd1a 0x10
#define MINOR_hd2a (MINOR_hd1a + NR_SUB_PER_PART)
#define MINOR_hd2c (MINOR_hd1a + NR_SUB_PER_PART + 2)

// 设备对应的硬盘号
#define DRV_OF_DEV(dev)                        \
    (dev <= MAX_PRIM ? dev / NR_PRIM_PER_DRIVE \
                     : (dev - MINOR_hd1a) / NR_SUB_PER_DRIVE)

/** 硬盘分区表, 包含4个16B的表项(如下)
 * 位于第一个扇区的0x1BE, 第一个扇区应该以55aa结尾 */
// 分区表项
typedef struct s_part_ent
{
    unsigned char active;     //是否可以引导(0x80)
    unsigned char start_head; // 起始磁头号
    unsigned char start_sector; // 起始扇区号(0-5), 起始柱面号高2位(6-7)
    unsigned char start_cyl; // 起始柱面号
    unsigned char sys_id; // 分区类型, 文件系统种类等, 是否是拓展分区
    unsigned char end_head;
    unsigned char end_sector;
    unsigned char end_cyl;
    unsigned int start_sect; //起始扇区的LBA
    unsigned int nr_sects;   //扇区数目
} PARTITION_ENTRY;

#define HD_PART_NO      0x00 /* unused entry */
#define HD_PART_EXT     0x05 /* extended partition */
#define HD_PART_ORANGES 0x99 /* Orange'S partition */

/***************/
/* DEFINITIONS */
/***************/
#define HD_TIMEOUT             10000 /* in millisec */
#define PARTITION_TABLE_OFFSET 0x1BE

#define SECTOR_SIZE       512
#define SECTOR_BITS       (SECTOR_SIZE * 8)
#define SECTOR_SIZE_SHIFT 9

#define P_PRIMARY  0
#define P_EXTENDED 1

typedef struct s_hd_request
{
    // int device;         // 设备号
    struct ata_cmd cmd;        // 存储需要执行的指令
    unsigned char *buf;        // 读/写的缓冲区
    int errors;                // 错误次数
    PROCESS *proc;             // 发出请求的进程
    struct s_hd_request *next; // 静态链表
} HD_REQUEST;

// 初始化硬盘驱动程序
void init_hd();


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

void rw_abs_hd(int rw, unsigned char drive, unsigned int lba,
               unsigned char nsector, unsigned char *buf);