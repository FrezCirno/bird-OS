#include <bird/bird.h>
#include <string.h>
#include <stdio.h>
#include <part.h>
#include <hd.h>

// 设备号    对应
// 0, 5     硬盘(2)
// 1-4, 6-9 主分区(8)
// 10-137   逻辑分区(8*16)
struct hd_info parts[MAX_DEV];

void probe_part()
{
    for (int i = 0; i < MAX_DEV; i++)
    {
        parts[i].base    = -1;
        parts[i].nsector = -1;
    }

    for (int i = 0; i < hd_num; i++)
    {
        int device    = DEV_OF_DRV(i);
        parts[device] = hd[i];
        probe_device(device);
    }
}

// dfs扫描分区
void probe_device(int device)
{
    int drive = DRV_OF_DEV(device);

    unsigned char buf[512];
    struct hd_info *this_part = &parts[device];
    struct part_info part_table[PART_NUM]; // 16 Bytes * PART_NUM

    printk("Probing device %d %x %x...", PEN_WHITE, device, this_part->base,
           this_part->nsector);
    // 获取分区表
    memset(buf, 0, 512);
    hd_rw(0, drive, this_part->base, 1, buf);
    if (!(buf[510] == 0x55 && buf[511] == 0xaa))
    {
        printk("probe_device: Device %d is not a extended part!\n", PEN_WHITE,
               device);
        return;
    }
    memcpy(part_table, buf + PARTITION_TABLE_OFFSET, sizeof(part_table));

    if (IS_DRV(device))
    {
        printk("Drive\n", PEN_WHITE);
        for (int i = 0; i < PART_NUM; i++)
        {
            int subdevice = device + i + 1;
            switch (part_table[i].sys_id)
            {
            case HD_PART_UNUSED: break;
            case HD_PART_EXT:
                printk("Find extended %d %x %x\n", PEN_WHITE, subdevice,
                       part_table[i].start_sect, part_table[i].nr_sects);
                parts[subdevice].base    = part_table[i].start_sect;
                parts[subdevice].nsector = part_table[i].nr_sects;
                probe_device(subdevice);
                break;
            default:
                printk("Find primary %d %x %x\n", PEN_WHITE, subdevice,
                       part_table[i].start_sect, part_table[i].nr_sects);
                parts[subdevice].base    = part_table[i].start_sect;
                parts[subdevice].nsector = part_table[i].nr_sects;
            }
        }
    }
    else if (IS_PART(device))
    {
        printk("Primary\n", PEN_WHITE);
        int subdevice            = PART_TO_LOG(device);
        parts[subdevice].base    = this_part->base + part_table[0].start_sect;
        parts[subdevice].nsector = part_table[0].nr_sects;
        printk("Find logical %d %x %x\n", PEN_WHITE, subdevice,
               parts[subdevice].base, parts[subdevice].nsector);
        if (part_table[1].sys_id != HD_PART_UNUSED) probe_device(subdevice);
    }
    else // IS_LOG
    {
        printk("Logical\n", PEN_WHITE);
        int subdevice            = device + 1;
        parts[subdevice].base    = this_part->base + part_table[0].start_sect;
        parts[subdevice].nsector = part_table[0].nr_sects;
        printk("Find logical %d %x %x\n", PEN_WHITE, subdevice,
               parts[subdevice].base, parts[subdevice].nsector);
        if (part_table[1].sys_id != HD_PART_UNUSED) probe_device(subdevice);
    }
}
