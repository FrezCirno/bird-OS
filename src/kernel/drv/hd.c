#include <asm/io.h>
#include <bird/sys.h>
#include <bird/bird.h> // panic
#include <bird/proc.h>
#include <string.h> // memcpy
#include <glib.h>
#include <int.h>
#include <hd.h>

/* main drive struct, one entry per drive */
unsigned char hd_num;
struct hd_info hd[2];

#define NR_REQUESTS 32 // 请求队列的长度

void (*hd_callback)(void);        // 发出读/写请求之后的回调
HD_REQUEST requests[NR_REQUESTS]; // 硬盘请求列表
PROCESS *wait_queue;              // 当requests队列满的时候的等待区
HD_REQUEST *this_request;         // 当前发出请求, 在回调中使用

unsigned char buf[512];

#define port_read(port, buf, nword) \
    __asm__("cld;rep insw" ::"d"(port), "D"(buf), "c"(nword))

#define port_write(port, buf, nword) \
    __asm__("cld;rep outsw" ::"d"(port), "S"(buf), "c"(nword))

void init_hd()
{
    /* Get the number of drives from the BIOS data area */
    hd_num = *(unsigned char *)(0x475);
    printk("HD Drives: %d\n", PEN_WHITE, hd_num);
    for (unsigned char i = 0; i < hd_num; i++)
    {
        hd_identify(i);
    }

    for (int i = 0; i < NR_REQUESTS; i++)
    {
        requests[i].cmd.drive = -1;
        requests[i].next      = NULL;
    }
    hd_callback = NULL;

    put_irq_handler(INT_VECTOR_IRQ_AT, hd_handler);
    enable_irq(INT_VECTOR_IRQ_AT);
    enable_irq(INT_VECTOR_IRQ_2);
}

void hd_identify(int drive)
{
    struct ata_cmd cmd = {
        .drive    = drive,
        .features = 0,
        .uselba   = 0,
        .lba      = 0,
        .nsector  = 0,
        .command  = ATA_CMD_IDENTIFY,
    };
    hd_sendcmd(&cmd, NULL);

    unsigned char isexist = in8(ATA_PORT_PRIM_STATUS_I);
    if (!isexist) return; // drive not exists
    if (hd_busy()) return;
    if (in8(ATA_PORT_PRIM_STATUS_I) & ATA_STATUS_ERR) return;

    port_read(ATA_PORT_PRIM_DATA_IO, buf, 256);

    print_identify_info((unsigned short *)buf);

    unsigned short *hdinfo = (unsigned short *)buf;

    /* Total Nr of User Addressable Sectors */
    hd[drive].base    = 0;
    hd[drive].nsector = ((int)hdinfo[61] << 16) + hdinfo[60];
}

void print_identify_info(unsigned short *hdinfo)
{
    char strbuf[512];
    char s[64];

    struct iden_info_ascii
    {
        int idx;
        int len;
        char *desc;
    } iinfo[] = {{10, 20, "HD SN"}, /* Serial number in ASCII */
                 {27, 40, "HD Model"} /* Model number in ASCII */};

    for (int k = 0; k < sizeof(iinfo) / sizeof(iinfo[0]); k++)
    {
        int i   = 0;
        char *p = (char *)&hdinfo[iinfo[k].idx];
        for (i = 0; i < iinfo[k].len / 2; i++)
        {
            s[i * 2 + 1] = *p++;
            s[i * 2]     = *p++;
        }
        s[i * 2] = 0;
        printk("HD count: %s: %s\n", PEN_WHITE, iinfo[k].desc, s);
    }

    int capabilities = hdinfo[49];
    printk("LBA supported: %s\n", PEN_WHITE,
           ((capabilities & 0x0200) ? "Yes" : "No"));

    int cmd_set_supported = hdinfo[83];
    printk("LBA48 supported: %s\n", PEN_WHITE,
           ((cmd_set_supported & 0x0400) ? "Yes" : "No"));

    int sectors = ((int)hdinfo[61] << 16) + hdinfo[60];
    printk("HD size: %dMB\n", PEN_WHITE, sectors * 512 / 1000000);
}

void hd_handler(unsigned int irq)
{
    printk("HD interrupt\n", PEN_BLUE);
    if (hd_callback != NULL)
    {
        void (*cb)(void) = hd_callback;
        hd_callback      = NULL;
        cb();
    }
}

void hd_reset(int drive)
{
    int i;

    out8(ATA_PORT_PRIM_CTRL_O, ATA_CTRL_SRST); // 重置该总线上的两块硬盘

    for (int i = 0; i < 1000; i++)
        ; // 等待一段时间.

    out8(ATA_PORT_PRIM_CTRL_O, 0); // 然后清除该位

    if (hd_busy()) panic("hd_reset: still busy");

    if (in8(ATA_PORT_PRIM_STATUS_I) == ATA_STATUS_ERR)
        panic("hd_reset: controller reset failed");

    struct ata_cmd rstcmd = {
        .drive   = drive,
        .uselba  = 1,
        .lba     = hd[0].base,
        .nsector = hd[0].nsector,
        .command = ATA_CMD_INIT,
    };
    hd_sendcmd(&rstcmd, hd_process);
}

void add_request(HD_REQUEST *req)
{
    req->next = NULL;
    cli();
    if (!this_request)
    {
        // 硬盘空闲, 立即处理
        this_request = req;
        sti();
        hd_process(); // 硬盘空闲说明一定没有中断在运行, 重新启动处理进程
        return;
    }
    // 否则加入请求队列中
    for (int i = 0; i < NR_REQUESTS; i++)
    {
        if (requests[i].cmd.drive == -1)
        {
            requests[i] = *req;
        }
    }
    sti();
    if (!hd_callback) hd_process();
}

void hd_process()
{
    printk("HD Process\n", PEN_LIGHT_GRAY);
    // 没有请求, 今日无事
    if (!this_request)
    {
        hd_callback = NULL;
        return;
    }
    if (this_request->cmd.command == ATA_CMD_WT_SEC)
    {
        hd_sendcmd(&this_request->cmd, write_intr); // 只发过命令不会有回应
        int r;
        for (int i = 0;
             i < 3000 && !(r = in8(ATA_PORT_PRIM_STATUS_I) & ATA_STATUS_DRQ); i++)
            continue;
        if (!r)
        {
            // 10ms 无DRQ, 考虑是否放弃
            bad_rw_intr();
            return;
        }
        port_write(ATA_PORT_PRIM_DATA_IO, this_request->buf, 256);
        // 写完数据之后, 等待中断
    }
    else if (this_request->cmd.command == ATA_CMD_RD_SEC)
    {
        hd_sendcmd(&this_request->cmd, read_intr);
    }
    else
    {
        panic("hd_process: Pardon? What command?");
    }
}

void hd_sendcmd(struct ata_cmd *cmd, void (*int_callback)(void))
{
    printk("Send cmd for %x\n", PEN_LIGHT_GREEN, cmd->lba);
    hd_callback = int_callback;
    ata_send_cmd(cmd);
}

int hd_cmd_ok()
{
    int i = in8(ATA_PORT_PRIM_STATUS_I);
    if ((i
         & (ATA_STATUS_RDY | ATA_STATUS_DRQ | ATA_STATUS_BSY | ATA_STATUS_ERR
            | ATA_STATUS_DFSE))
        == (ATA_STATUS_RDY | ATA_STATUS_DRQ))
        return 1;
    if (i & ATA_STATUS_ERR) i = in8(ATA_PORT_PRIM_ERROR_I);
    return 0;
}

int hd_busy()
{
    unsigned int i;

    // wait until BSY=0 or timeout
    for (int k = 0; k < 100000; k++)
    {
        i = in8(ATA_PORT_PRIM_STATUS_I);
        if ((i & (ATA_STATUS_BSY | ATA_STATUS_RDY)) == ATA_STATUS_RDY)
        {
            break; // BSY=0, RDY=1
        }
    }

    i = in8(ATA_PORT_PRIM_STATUS_I);
    i &= (ATA_STATUS_BSY | ATA_STATUS_RDY | ATA_STATUS_DRQ);
    if ((i & ATA_STATUS_RDY) | (i | ATA_STATUS_DRQ))
    {
        return 0; // BSY=0, RDY/DRQ=1
    }
    return 1; // BSY=1
}

void bad_rw_intr()
{
    printk("HD failed\n", PEN_RED);
    if (++this_request->errors >= 7) hd_request_fin();
    hd_reset(this_request->cmd.drive);
}

void read_intr()
{
    if (!hd_cmd_ok())
    {
        bad_rw_intr();
        return;
    }
    port_read(ATA_PORT_PRIM_DATA_IO, this_request->buf, 256);
    this_request->errors = 0;
    this_request->buf += 512;

    if (--this_request->cmd.nsector)
    {
        hd_callback = read_intr;
        return;
    }
    hd_request_fin();
    // 不让硬盘空闲, 看看有没有下一个任务
    hd_process();
}

void write_intr()
{
    // 单个扇区写入完毕
    if (!hd_cmd_ok())
    {
        bad_rw_intr();
        return;
    }

    this_request->errors = 0;
    this_request->buf += 512;

    if (--this_request->cmd.nsector)
    {
        // 再写一个扇区, 设置回调
        port_write(ATA_PORT_PRIM_DATA_IO, this_request->buf, 256);
        hd_callback = write_intr;
        return;
    }
    hd_request_fin();
    // 不让硬盘空闲, 看看有没有下一个任务
    hd_process();
}
void lock_request(HD_REQUEST *req)
{
    req->lock = 1;
}
void unlock_request(HD_REQUEST *req)
{
    if (!req->lock) printk("hd.c: free buffer being unlocked\n", PEN_RED);
    req->lock = 0;
    wake_up(&req->proc);
}
void wait_request(HD_REQUEST *req)
{
    cli();
    while (req->lock) sleep_on(&req->proc);
    sti();
}
void hd_request_fin()
{
    // 完毕, 检查下一个任务
    printk("request for %x fin.\n", PEN_BLUE, this_request->cmd.lba);
    wake_up(&wait_queue);
    unlock_request(this_request);
    cli();
    int i;
    for (i = 0; i < 31; i++)
    {
        if (requests[i].cmd.drive == -1) break;
        requests[i] = requests[i + 1];
    }
    requests[i].cmd.drive = -1;
    if (requests[0].cmd.drive != -1)
        this_request = requests;
    else
        this_request = NULL;
    sti();
    hd_process();
}

void hd_rw(int rw, unsigned char drive, unsigned int lba, unsigned char nsector,
           unsigned char *buf)
{
    HD_REQUEST *req;

    if (rw != 0 && rw != 1) panic("hd_rw: Bad hd command, must be R/W");

repeat:
    for (req = requests; req < requests + NR_REQUESTS; req++)
        if (req->cmd.drive < 0) break;
    if (req == requests + NR_REQUESTS)
    {
        sleep_on(&wait_queue);
        goto repeat;
    }

    lock_request(req);

    req->buf          = buf;
    req->cmd.drive    = drive;
    req->cmd.uselba   = 1;
    req->cmd.lba      = lba;
    req->cmd.nsector  = nsector;
    req->cmd.features = 0;
    req->cmd.command  = ((rw == 0) ? ATA_CMD_RD_SEC : ATA_CMD_WT_SEC);
    req->errors       = 0;
    req->next         = NULL;

    add_request(req);
    wait_request(req);
}
