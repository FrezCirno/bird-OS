#include <asm/io.h>
#include <bird/sys.h>
#include <bird/bird.h> // panic
#include <bird/proc.h>
#include <string.h> // memcpy
#include <int.h>
#include <hd.h>

/* main drive struct, one entry per drive */
struct hd_struct
{
    struct
    {
        unsigned int base;    //起始扇区LBA
        unsigned int nsector; //扇区数目
    } primary[NR_PRIM_PER_DRIVE], logical[NR_SUB_PER_DRIVE];
} hd[1]; // 只支持一个drive

#define NR_REQUESTS 32

void (*hd_callback)(void);        // 发出读/写请求之后的回调
HD_REQUEST requests[NR_REQUESTS]; // 硬盘请求列表
HD_REQUEST *this_request;         // 当前发出请求, 在回调中使用

#define port_read(port, buf, nword) \
    __asm__("cld;rep insw" ::"d"(port), "D"(buf), "c"(nword))

#define port_write(port, buf, nword) \
    __asm__("cld;rep outsw" ::"d"(port), "S"(buf), "c"(nword))

void init_hd()
{
    /* Get the number of drives from the BIOS data area */
    unsigned char *pNrDrives = (unsigned char *)(0x475);

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

void hd_handler(unsigned int irq)
{
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
    out8(ATA_PORT_PRIM_CMD_O, 4);

    for (int i = 0; i < 1000; i++) nop(); // 等待一段时间.

    out8(ATA_PORT_PRIM_CMD_O, ATA_CMD_NOP);

    if (hd_busy()) panic("hd_reset: still busy");

    if (in8(ATA_PORT_SECO_STATUS_I) == ATA_STATUS_ERR)
        panic("hd_reset: controller reset failed");

    struct ata_cmd rstcmd = {
        .drive   = drive,
        .lba     = hd[0].primary[0].base,
        .nsector = hd[0].primary[0].nsector,
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
        hd_process();
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
    // 没有请求, 今日无事
    if (!this_request)
    {
        hd_callback = NULL;
        return;
    }
    if (this_request->cmd.command == ATA_CMD_WT_SEC)
    {
        hd_sendcmd(&this_request->cmd, write_intr);
        int r;
        for (int i = 0;
             i < 100000 && !(r = in8(ATA_PORT_PRIM_STATUS_I) & ATA_STATUS_DRQ);
             i++)
            continue;
        if (!r)
        {
            // 10ms 无DRQ, 考虑是否放弃
            bad_rw_intr();
            return;
        }
        port_write(ATA_PORT_PRIM_DATA_IO, this_request->buf, 256);
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
    for (i = 0; i < 10000; i++)
    {
        i = in8(ATA_PORT_PRIM_STATUS_I);
        if ((i & (ATA_STATUS_BSY | ATA_STATUS_RDY)) == ATA_STATUS_RDY) break;
    }
    i = in8(ATA_PORT_PRIM_STATUS_I);

    if ((i & ATA_STATUS_BSY | ATA_STATUS_RDY | ATA_STATUS_DRQ)
        == (ATA_STATUS_RDY | ATA_STATUS_DRQ))
        return 0;
    return 1;
}

void bad_rw_intr()
{
    if (++this_request->errors >= 7) hd_request_fin();
    if (this_request->errors > 3) hd_reset(this_request->cmd.drive);
}

void read_intr()
{
    if (!hd_cmd_ok())
    {
        bad_rw_intr();
        hd_process();
        return;
    }
    port_read(ATA_PORT_PRIM_DATA_IO, this_request->buf, 256);
    this_request->errors = 0;
    this_request->buf += 512;
    this_request->cmd.lba++;

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
    if (!hd_cmd_ok())
    {
        bad_rw_intr();
        hd_process();
        return;
    }

    if (--this_request->cmd.nsector)
    {
        port_write(ATA_PORT_PRIM_DATA_IO, this_request->buf, 256);
        return;
    }
    hd_request_fin();
    // 不让硬盘空闲, 看看有没有下一个任务
    hd_process();
}

void hd_request_fin()
{
    // 完毕, 检查下一个任务
    wake_up(&this_request->proc);
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
}

void rw_abs_hd(int rw, unsigned char drive, unsigned int lba,
               unsigned char nsector, unsigned char *buf)
{
    HD_REQUEST *req;

    if (rw != 0 && rw != 1) panic("rw_abs_hd: Bad hd command, must be R/W");
repeat:
    for (req = requests; req < requests + NR_REQUESTS; req++)
        if (req->cmd.drive < 0) break;
    if (req == requests + NR_REQUESTS) goto repeat;

    req->buf          = buf;
    req->cmd.drive    = drive;
    req->cmd.lba      = lba;
    req->cmd.nsector  = nsector;
    req->cmd.features = 0;
    req->cmd.command  = ((rw == 0) ? ATA_CMD_RD_SEC : ATA_CMD_WT_SEC);
    req->errors       = 0;
    req->next         = NULL;
    add_request(req);
    sleep_on(&req->proc);
}
