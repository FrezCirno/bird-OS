#pragma once
#include <hd.h>
// 硬盘分区表, 包含4个16B的表项(如下)
// 位于第一个扇区的0x1BE, 第一个扇区应该以55aa结尾
// 分区表项
struct part_info
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
};

#define HD_PART_UNUSED  0x00 /* unused entry */
#define HD_PART_EXT     0x05 /* extended partition */
#define HD_PART_ORANGES 0x99 /* Orange'S partition */

#define MAX_IO_BYTES 256 /* how many sectors does one IO can handle */

// linux编号规则
// 主设备号: 表示设备是硬盘/软盘/内存盘, 如sda, sdb, fda, ...
// 次设备号: 用来区别分区
// 每个设备的主分区表直接指明的分区用1-4表示, 可能不全用到, 如sda1, sda2, ...
// 每个拓展分区下细分的逻辑分区用5之后的数字表示, 如sda5, sda6, ...
// 举例1: sda = (sda1, sda2, sda3, [sda5, sda6, sda7])
// 举例2: sda = (sda1, sda2, [sda5, sda6], [sda7, sda8, sda9])
// 举例3: sda = (sda1, [sda5, sda6, sda7, sda8, sda9],,)
// 不过这里采用了oranges的命名规则

// ATA/IDE接口: >=2个 - Primary和Secondary, 分别有两套IO端口
// 每个接口可以连接两个设备: Master和Slave (由ATA_CMD_DRIVE标志区分)
#define MAX_DRIVES 2 // 最大设备数(hda, hdb)(这里只考虑Primary, 因此是2)
// 每个设备的主分区表最大分区(PART)数(主分区+拓展分区)
#define PART_NUM 4
// 每个拓展分区的最大逻辑分区数(链表, 理论上可以超大)
#define MAX_LOG_PER_PART 16
// 每个设备的最大逻辑分区数(主分区表的4个分区全是拓展分区, 每个拓展分区都有16个逻辑分区)
#define MAX_LOG_PER_DRIVE (MAX_LOG_PER_PART * PART_NUM)

// 整块硬盘和主分区(PART)放在一起编号和存储, 称为PRIM
// 类似 hd0 = (hd1[...], hd2[...], hd3[...], hd4[...])
// 每块硬盘的PRIM数量(5)
#define MAX_PRIM_PER_DRIVE (PART_NUM + 1)

// 硬盘+主分区数量, 即
// hd0 = (hd1, hd2, hd3, hd4), hd5 = (hd6, hd7, hd8, hd9)
#define MAX_PRIMS (MAX_DRIVES * MAX_PRIM_PER_DRIVE)

// 在Oranges中, 逻辑分区编号自成一体, 例
// hd1 = [x, x+1, x+2, ..., x+16], hd2 = [x+17, x+18, ..., x+32]
// 逻辑分区编号从10开始
#define LOG_START MAX_PRIMS

// 设备号dev  表示
// 0, 5      硬盘drive(2) -- prim
// 1-4, 6-9  主分区part(8) /
// 10-137    逻辑分区(8*16) -- log
#define IS_PRIM(dev) ((dev) < LOG_START)
#define IS_DRV(dev)  (IS_PRIM(dev) && (((dev) % MAX_PRIM_PER_DRIVE) == 0))
#define IS_PART(dev) (IS_PRIM(dev) && !IS_DRV(dev))
#define IS_LOG(dev)  ((dev) >= LOG_START)

// 设备号(0-137)对应的硬盘号(0/1)
// 如果设备号小于LOG_START, 说明此设备号表示整块硬盘或者一个主分区
// 否则表示一个逻辑分区
#define DRV_OF_DEV(dev)                          \
    (IS_PRIM(dev) ? ((dev) / MAX_PRIM_PER_DRIVE) \
                  : (((dev)-LOG_START) / MAX_LOG_PER_DRIVE))

// 硬盘号(0/1)对应的设备号(0,5)
#define DEV_OF_DRV(drive) ((drive)*MAX_PRIM_PER_DRIVE)

// 主分区设备号(1-4,6-9,...)下属的逻辑分区的起始设备号
#define PART_TO_LOG(part) \
    (((part)-DRV_OF_DEV(part) - 1) * MAX_LOG_PER_PART + LOG_START)

// 逻辑分区设备号所属的主分区设备号
#define PART_OF_LOG(log) \
    (((log)-LOG_START) / MAX_LOG_PER_DRIVE + DRV_OF_DEV(log) + 1)

#define PARTITION_TABLE_OFFSET 0x1BE

#define MAX_DEV (MAX_PRIMS + MAX_LOG_PER_DRIVE)

extern struct hd_info parts[MAX_DEV];

void probe_part();

// dfs扫描分区
void probe_device(int device);