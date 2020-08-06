#include <io.h>
#include <int.h>
#include <buffer.h>
#include <glib.h>
#include <string.h>
#include <mouse.h>

#define MOUSE_BUF_SIZE 128

u8 mouse_buf[MOUSE_BUF_SIZE];
FIFO_BUFFER mouse_in;

int mouse_x, mouse_y;
MOUSE_DEC mdec;
SHEET *mouse_sht;
u8 mouse_sht_buf[8 * 16];

void wait_KBC_sendready()
{
    /* 等待键盘控制电路准备完毕 */
    for (;;)
        if ((in8(MOUSE_PORT_CMD) & MOUSE_NOT_READY) == 0) break;
}

void init_mouse()
{
    /* 初始化键盘控制电路 */
    wait_KBC_sendready();
    out8(MOUSE_PORT_CMD, MOUSE_WRITE_MODE);
    wait_KBC_sendready();
    out8(MOUSE_PORT_DATA, KBC_MODE);
    /* 激活鼠标 */
    wait_KBC_sendready();
    out8(MOUSE_PORT_CMD, KEYCMD_SENDTO_MOUSE);
    wait_KBC_sendready();
    out8(MOUSE_PORT_DATA, MOUSE_ENABLE);
    /* 顺利的话，键盘控制器会返回ACK(0xfa) */

    mouse_x    = scr_x / 2;
    mouse_y    = scr_y / 2;
    mdec.phase = 0;
    fifo_init(&mouse_in, MOUSE_BUF_SIZE, mouse_buf);

    mouse_sht = sheet_alloc();
    sheet_setbuf(mouse_sht, mouse_sht_buf, 8, 16, 255); /* 透明色号255 */
    fillRectTo(mouse_sht_buf, 8, 0, 0, 8, 16, 255);
    drawGlyphTo(mouse_sht_buf, 8, 0, 0, cursor, PEN_WHITE);

    sheet_slide(mouse_sht, mouse_x, mouse_y);

    sheet_updown(mouse_sht, 2);

    put_irq_handler(INT_VECTOR_IRQ_MOUSE, mouse_handler);
    enable_irq(INT_VECTOR_IRQ_MOUSE);
}

void mouse_handler(u32 irq)
{
    fifo_push(&mouse_in, in8(MOUSE_PORT_DATA));
}

int mouse_decode(u8 dat)
{
    switch (mdec.phase)
    {
    case 0:
        /* 等待鼠标的0xfa的阶段 */
        if (dat == 0xfa)
        {
            mdec.phase = 1;
        }
        return 0;
    case 1:
        /* 等待鼠标第一字节的阶段 */
        if ((dat & 0xc8) == 0x08)
        {
            mdec.buf[0] = dat;
            mdec.phase  = 2;
        }
        return 0;
    case 2:
        /* 等待鼠标第二字节的阶段 */
        mdec.buf[1] = dat;
        mdec.phase  = 3;
        return 0;
    case 3:
        /* 等待鼠标第二字节的阶段 */
        mdec.buf[2] = dat;
        mdec.phase  = 1;
        mdec.btn    = mdec.buf[0] & 0x07;
        mdec.x      = mdec.buf[1];
        mdec.y      = mdec.buf[2];
        if ((mdec.buf[0] & 0x10) != 0)
        {
            mdec.x |= 0xffffff00;
        }
        if ((mdec.buf[0] & 0x20) != 0)
        {
            mdec.y |= 0xffffff00;
        }
        /* 鼠标的y方向与画面符号相反 */
        mdec.y = -mdec.y;
        return 1;
    default: return -1;
    }
}

void mouse_read()
{
    if (mouse_in.size)
    {
        io_cli();
        int i = fifo_pop(&mouse_in);
        io_sti();

        if (mouse_decode(i) != 0)
        {
            /* 鼠标指针的移动 */
            mouse_x += mdec.x;
            mouse_y += mdec.y;
            if (mouse_x < 0) mouse_x = 0;
            if (mouse_y < 0) mouse_y = 0;
            if (mouse_x >= scr_x) mouse_x = scr_x - 1;
            if (mouse_y >= scr_y) mouse_y = scr_y - 1;
            sheet_slide(mouse_sht, mouse_x, mouse_y);
        }
    }
}