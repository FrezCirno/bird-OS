#include <asm/io.h>
#include "ata.h"

unsigned char cur_drive;

#define ATA_MAKE_DRIVE(use_lba, drive, lba3)                           \
    ((use_lba ? ATA_DRIVE_LBA : 0xA0) | (drive ? ATA_DRIVE_DRV : 0xA0) \
     | (lba3 & 0xF))

unsigned char ata_read_status(unsigned char drive)
{
    register unsigned char stat;
    if (cur_drive != drive)
    {
        cur_drive = drive;
        out8(ATA_PORT_PRIM_DRIVE_IO, drive);
        // port in 5 times instead of wait
        __asm__("inb %%dx, %%al\n\t"
                "inb %%dx, %%al\n\t"
                "inb %%dx, %%al\n\t"
                "inb %%dx, %%al\n\t"
                "inb %%dx, %%al"
                : "=a"(stat)
                : "d"(ATA_PORT_PRIM_STATUS_I));
    }
    else
    {
        stat = in8(ATA_PORT_PRIM_STATUS_I);
    }
    return stat;
}

void ata_send_cmd(struct ata_cmd *cmd)
{
    out8(ATA_PORT_PRIM_CTRL_O, 0); // 打开硬盘中断
    out8(ATA_PORT_PRIM_FEATURES_O, cmd->features);
    out8(ATA_PORT_PRIM_NSECTOR_IO, cmd->nsector);
    out8(ATA_PORT_PRIM_LBA0_IO, cmd->lba & 0xff);
    out8(ATA_PORT_PRIM_LBA1_IO, cmd->lba >> 8 & 0xff);
    out8(ATA_PORT_PRIM_LBA2_IO, cmd->lba >> 16 & 0xff);
    out8(ATA_PORT_PRIM_DRIVE_IO,
         ATA_MAKE_DRIVE(cmd->uselba, cmd->drive, cmd->lba >> 24 & 0xf));
    out8(ATA_PORT_PRIM_CMD_O, cmd->command);
}
