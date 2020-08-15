#pragma once

#define ATA_PORT_PRIM_DATA_IO    0x1F0 // Read/Write PIO data bytes
#define ATA_PORT_PRIM_FEATURES_O 0x1F1 // Command specific interface features
#define ATA_PORT_PRIM_ERROR_I    0x1F1 // Error of last cmd
#define ATA_PORT_PRIM_NSECTOR_IO 0x1F2 // Num of sectors to r/w
#define ATA_PORT_PRIM_LBA0_IO    0x1F3
#define ATA_PORT_PRIM_LBA1_IO    0x1F4
#define ATA_PORT_PRIM_LBA2_IO    0x1F5
#define ATA_PORT_PRIM_DRIVE_IO   0x1F6 // Select a drive, has extra flags bit
#define ATA_PORT_PRIM_STATUS_I   0x1F7 // Read current status
#define ATA_PORT_PRIM_CMD_O      0x1F7 // Send the cmd
#define ATA_PORT_PRIM_CTRL_O     0x3F6 // Reset the bus or enable/disable int
#define ATA_PORT_PRIM_STATUS2_I  0x3F6 // 从这里读取状态不会重置中断位
#define ATA_PORT_PRIM_DRV_ADDR_I 0x3F7 // Provide some drive select info

#define ATA_PORT_SECO_DATA_IO    0x170 // Read/Write PIO data bytes
#define ATA_PORT_SECO_FEATURES_O 0x171 // Command specific interface features
#define ATA_PORT_SECO_ERROR_I    0x171 // Error of last cmd
#define ATA_PORT_SECO_NSECTOR_IO 0x172 // Num of sectors to r/w
#define ATA_PORT_SECO_LBA0_IO    0x173
#define ATA_PORT_SECO_LBA1_IO    0x174
#define ATA_PORT_SECO_LBA2_IO    0x175
#define ATA_PORT_SECO_DRIVE_IO   0x176 // Select a drive, has extra flags bit
#define ATA_PORT_SECO_STATUS_I   0x177 // Read current status
#define ATA_PORT_SECO_CMD_O      0x177 // Send the cmd
#define ATA_PORT_SECO_CTRL_O     0x376 // Reset the bus or enable/disable int
#define ATA_PORT_SECO_STATUS2_I  0x376 // 从这里读取状态不会重置中断位
#define ATA_PORT_SECO_DRV_ADDR_I 0x377 // Provide some drive select info

#define ATA_ERROR_AMNF  0x1  // Address mark not found.
#define ATA_ERROR_TKZNF 0x2  // Track zero not found.
#define ATA_ERROR_ABRT  0x4  // Aborted command.
#define ATA_ERROR_MCR   0x8  // Media change request.
#define ATA_ERROR_IDNF  0x10 // ID not found.
#define ATA_ERROR_MC    0x20 // Media changed.
#define ATA_ERROR_UNC   0x40 // Uncorrectable data error.
#define ATA_ERROR_BBK   0x80 // Bad Block detected.

#define ATA_DRIVE_LBA3     (0xA0 | 0xf)  // LBA模式高4位
#define ATA_DRIVE_CHS_HEAD (0xA0 | 0xf)  // CHS模式磁头号
#define ATA_DRIVE_DRV      (0xA0 | 0x10) // 硬盘号, 0主盘, 1副盘
#define ATA_DRIVE_LBA      (0xA0 | 0x40) // 使用LBA 而非CHS

#define ATA_STATUS_ERR  0x01 // An error occurred, send new cmd/rst to clear it
#define ATA_STATUS_IDX  0x02 // Index, always 0
#define ATA_STATUS_CORR 0x04 // Corrected data, always 0
#define ATA_STATUS_DRQ  0x08 // Ready to transfer data, IN/OUT
#define ATA_STATUS_SRV  0x10 // Command dependent. (formerly DSC bit)
#define ATA_STATUS_DFSE 0x20 // Device Fault / Stream Error (does not set ERR)
#define ATA_STATUS_RDY  0x40 // Drive Ready
#define ATA_STATUS_BSY  0x80 // You'd wait for it to clear

#define ATA_CMD_NOP      0x0
#define ATA_CMD_RESTORE  0x10 // 驱动器重新校正（驱动器复位）。
#define ATA_CMD_RD_SEC   0x20 // Read Sectors with Retry
#define ATA_CMD_WT_SEC   0x30 // Write Sectors with Retry
#define ATA_CMD_FORMAT   0x50 // 格式化磁道。
#define ATA_CMD_SEEK     0x70 // 寻道。
#define ATA_CMD_DIAGNOSE 0x90 // 控制器诊断。
#define ATA_CMD_INIT     0x91 // Initialize Drive Parameters
#define ATA_CMD_IDENTIFY 0xEC // Identify Device

#define ATA_CTRL_INIT 0   // Set this first
#define ATA_CTRL_nIEN 0x2 // Stop sending interrupt
// Set, then clear (after 5us), this to do a "Software Reset" on all ATA drives
// on a bus, if one is misbehaving.
#define ATA_CTRL_SRST 0x4
#define ATA_CTRL_HOB  0x80 // High Order Byte (LBA48模式使用)

#define ATA_DRV_ADDR_DS0 0x1 // Drive 0 select. Clears when drive 0 selected.
#define ATA_DRV_ADDR_DS1 0x2 // Drive 1 select. Clears when drive 1 selected.
//	One's compliment representation of the currently selected head.
#define ATA_DRV_ADDR_HS0 0x4
#define ATA_DRV_ADDR_HS1 0x8
#define ATA_DRV_ADDR_HS2 0x10
#define ATA_DRV_ADDR_HS3 0x20
// Write gate; goes low while writing to the drive is in progress.
#define ATA_DRV_ADDR_WTG 0x40

struct ata_cmd
{
    unsigned char features;
    unsigned char nsector;
    unsigned char uselba;
    unsigned int lba; // lba地址, 28位, 会自动转换
    char drive;       // 硬盘号, 0/1
    unsigned char command;
};

unsigned char ata_read_status(unsigned char drive);

void ata_send_cmd(struct ata_cmd *cmd);