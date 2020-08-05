#include <types.h>
#include <io.h>
#include <glib.h>
#include <string.h>
#include <font.h>
#include <memory.h>

u8 *vram = (u8 *)0xa0000;
u32 scr_x;
u32 scr_y;
u32 scr_bpp;
u32 scr_pitch;

u32 cur_x;
u32 cur_y;

SHTCTL *ctl;

// 每个bank 0xa0000-0xb0000 共 0x10000 = 64K 字节
// 显存一共800*600共 480000 字节
// 8个bank
// bank n offset m <-> (n << 16) | m <-> 0xa0000 | m

u32 fontsmap[65536];

const u8 cursor[16] = {0b10000000, 0b11000000, 0b11100000, 0b11110000,
                       0b11111000, 0b11111100, 0b11111110, 0b11111111,
                       0b11111000, 0b11011000, 0b00001100, 0b00000110,
                       0b00000011};

void init_video()
{
    scr_x     = 800;
    scr_y     = 600;
    scr_bpp   = 8;
    scr_pitch = scr_x * scr_bpp / 8;

    bga_set_video_mode(scr_x, scr_y, scr_bpp, 0, 0);
    bga_set_bank(0);
    initPalette();
    cacheFonts();
    shtctl_init(scr_x, scr_y);
}

void initPalette()
{
    static const u8 rgb[48] = {
        0x00, 0x00, 0x00, /*  0:黑 */
        0xff, 0x00, 0x00, /*  1:梁红 */
        0x00, 0xff, 0x00, /*  2:亮绿 */
        0xff, 0xff, 0x00, /*  3:亮黄 */
        0x00, 0x00, 0xff, /*  4:亮蓝 */
        0xff, 0x00, 0xff, /*  5:亮紫 */
        0x00, 0xff, 0xff, /*  6:浅亮蓝 */
        0xff, 0xff, 0xff, /*  7:白 */
        0xc6, 0xc6, 0xc6, /*  8:亮灰 */
        0x84, 0x00, 0x00, /*  9:暗红 */
        0x00, 0x84, 0x00, /* 10:暗绿 */
        0x84, 0x84, 0x00, /* 11:暗黄 */
        0x00, 0x00, 0x84, /* 12:暗青 */
        0x84, 0x00, 0x84, /* 13:暗紫 */
        0x00, 0x84, 0x84, /* 14:浅暗蓝 */
        0x84, 0x84, 0x84  /* 15:暗灰 */
    };
    setPalette(0, 15, rgb);
}

void setPalette(int start, int end, const u8 *palette)
{
    u32 eflags = io_load_eflags();
    io_cli();
    out8(0x03c8, start);
    for (int i = start; i <= end; i++)
    {
        out8(0x03c9, palette[0] / 4);
        out8(0x03c9, palette[1] / 4);
        out8(0x03c9, palette[2] / 4);
        palette += 3;
    }
    io_store_eflags(eflags);
}

void cacheFonts()
{
    for (u32 i = 0; i < fonts.Chars; i++)
    {
        u16 index       = fonts.Index[i];
        fontsmap[index] = i;
    }
}

void putPixelTo(u8 *dst, int pitch, int x, int y, u8 color)
{
    u32 base  = y * pitch + x;
    dst[base] = color;
}

void putPixel(int x, int y, u8 color)
{
    u32 base = y * scr_pitch + x;
    if (base < bank_start || base >= bank_end)
    {
        bga_set_bank(base >> 16);
    }
    vram[base & 0xffff] = color;
}

void fillRectTo(u8 *dst, int pitch, int x1, int y1, int x2, int y2, u8 color)
{
    for (int y = y1; y <= y2; y++)
    {
        for (int x = x1; x <= x2; x++)
        {
            putPixelTo(dst, pitch, x, y, color);
        }
    }
}

void fillRect(int x1, int y1, int x2, int y2, u8 color)
{
    for (int y = y1; y <= y2; y++)
    {
        for (int x = x1; x <= x2; x++)
        {
            putPixel(x, y, color);
        }
    }
}

void drawGlyphTo(u8 *dst, int pitch, int x, int y, const u8 *glyph, u8 color)
{
    u32 base = y * pitch + x;
    for (int in_y = 0; in_y < 16; in_y++)
    {
        u8 line     = glyph[in_y];
        u8 mask     = 0x80;
        u32 in_base = base;
        while (mask)
        {
            if (line & mask)
            {
                dst[in_base] = color;
            }
            in_base++;
            mask >>= 1;
        }
        base += pitch;
    }
}

void drawGlyph(int x, int y, const u8 *glyph, u8 color)
{
    u32 base = y * scr_pitch + x;
    for (int in_y = 0; in_y < fonts.Height; in_y++)
    {
        u8 line     = glyph[in_y];
        u8 mask     = 0x80;
        u32 in_base = base;
        while (mask)
        {
            if (line & mask)
            {
                if (in_base < bank_start || in_base >= bank_end)
                {
                    bga_set_bank(in_base >> 16);
                }
                vram[in_base & 0xffff] = color;
            }
            in_base++;
            mask >>= 1;
        }
        base += scr_pitch;
    }
}

void drawChar(int x, int y, char ch, u8 color)
{
    const u8 *font_data = &fonts.Bitmap[16 * fontsmap[ch]];
    drawGlyph(x, y, font_data, color);
}

void drawText(int x, int y, const char *str, u8 color)
{
    while (*str)
    {
        drawChar(x, y, *str++, color);
        x += 8;
    }
}

void gotoxy(u32 x, u32 y)
{
    cur_x = x;
    cur_y = y;
}

void putchar(char ch, u8 color)
{
    if (ch == '\n')
    {
        cur_x = 0;
        cur_y += fonts.Height;
    }
    else if (ch == '\t')
    {
        cur_x += 4 * fonts.Width;
    }
    else
    {
        drawChar(cur_x, cur_y, ch, color);
        cur_x += fonts.Width;
    }
    if (cur_x >= scr_x)
    {
        cur_x = 0;
        cur_y += fonts.Height;
    }
    if (cur_y >= scr_y)
    {
        cur_y = 0;
    }
}

void printstr(const char *str, u8 color)
{
    while (*str)
    {
        putchar(*str++, color);
    }
}

void shtctl_init(int x_size, int y_size)
{
    ctl = (SHTCTL *)mm_alloc_4k(sizeof(SHTCTL));
    if (ctl == 0)
    {
        return;
    }
    ctl->vram  = vram;
    ctl->xsize = x_size;
    ctl->ysize = y_size;
    ctl->top   = -1;
    for (int i = 0; i < MAX_SHEETS; i++)
    {
        ctl->_sheets[i].flags = 0;
    }
}

SHEET *sheet_alloc()
{
    for (int i = 0; i < MAX_SHEETS; i++)
    {
        if (ctl->_sheets[i].flags == 0)
        {
            SHEET *sht  = &ctl->_sheets[i];
            sht->flags  = SHEET_IN_USE; /* 标记为正在使用*/
            sht->height = -1;           /* 隐藏 */
            return sht;
        }
    }
    return 0; /* 所有的SHEET都处于正在使用状态*/
}

void sheet_setbuf(SHEET *sht, u8 *buf, int xsize, int ysize, int col_inv)
{
    sht->buf     = buf;
    sht->bxsize  = xsize;
    sht->bysize  = ysize;
    sht->col_inv = col_inv;
}

void sheet_updown(SHEET *sht, int height)
{
    int h, old = sht->height; /* 存储设置前的高度信息 */
    if (height > ctl->top + 1)
    {
        height = ctl->top + 1;
    }
    if (height < -1)
    {
        height = -1;
    }
    sht->height = height; /* 设定高度 */

    /* 下面主要是进行sheets[ ]的重新排列 */
    if (old > height)
    { /* 比以前低 */
        if (height >= 0)
        {
            /* 把中间的往上提 */
            for (h = old; h > height; h--)
            {
                ctl->sheets[h]         = ctl->sheets[h - 1];
                ctl->sheets[h]->height = h;
            }
            ctl->sheets[height] = sht;
        }
        else
        { /* 隐藏 */
            if (ctl->top > old)
            {
                /* 把上面的降下来 */
                for (h = old; h < ctl->top; h++)
                {
                    ctl->sheets[h]         = ctl->sheets[h + 1];
                    ctl->sheets[h]->height = h;
                }
            }
            ctl->top--; /* 由于显示中的图层减少了一个，所以最上面的图层高度下降 */
        }
        sheet_refreshsub(sht->vx0, sht->vy0, sht->vx0 + sht->bxsize,
                         sht->vy0 + sht->bysize); /* 按新图层的信息重新绘制画面 */
    }
    else if (old < height)
    { /* 比以前高 */
        if (old >= 0)
        {
            /* 把中间的拉下去 */
            for (h = old; h < height; h++)
            {
                ctl->sheets[h]         = ctl->sheets[h + 1];
                ctl->sheets[h]->height = h;
            }
            ctl->sheets[height] = sht;
        }
        else
        { /* 由隐藏状态转为显示状态 */
            /* 将已在上面的提上来 */
            for (h = ctl->top; h >= height; h--)
            {
                ctl->sheets[h + 1]         = ctl->sheets[h];
                ctl->sheets[h + 1]->height = h + 1;
            }
            ctl->sheets[height] = sht;
            ctl->top++; /* 由于已显示的图层增加了1个，所以最上面的图层高度增加 */
        }
        sheet_refreshsub(sht->vx0, sht->vy0, sht->vx0 + sht->bxsize,
                         sht->vy0 + sht->bysize); /* 按新图层信息重新绘制画面 */
    }
}

void sheet_refresh(SHEET *sht, int bx0, int by0, int bx1, int by1)
{
    if (sht->height >= 0)
    { /* 如果正在显示，则按新图层的信息刷新画面*/
        sheet_refreshsub(sht->vx0 + bx0, sht->vy0 + by0, sht->vx0 + bx1,
                         sht->vy0 + by1);
    }
}

void sheet_refreshsub(int vx0, int vy0, int vx1, int vy1)
{
    u8 *vram = ctl->vram;
    for (int h = 0; h <= ctl->top; h++)
    {
        SHEET *sht = ctl->sheets[h];
        u8 *buf    = sht->buf;
        /* 计算相对clipbox */
        int bx0 = vx0 - sht->vx0;
        int bx1 = vx1 - sht->vx0;
        int by0 = vy0 - sht->vy0;
        int by1 = vy1 - sht->vy0;
        if (bx0 < 0) bx0 = 0;
        if (by0 < 0) by0 = 0;
        if (bx1 > sht->bxsize) bx1 = sht->bxsize;
        if (by1 > sht->bysize) by1 = sht->bysize;

        for (int by = by0; by < by1; by++)
        {
            int vy = sht->vy0 + by;
            for (int bx = bx0; bx < bx1; bx++)
            {
                int vx   = sht->vx0 + bx;
                u8 color = buf[by * sht->bxsize + bx];
                if (color != sht->col_inv)
                {
                    putPixel(vx, vy, color);
                }
            }
        }
    }
}

void sheet_slide(SHEET *sht, int vx0, int vy0)
{
    int old_vx0 = sht->vx0, old_vy0 = sht->vy0;
    sht->vx0 = vx0;
    sht->vy0 = vy0;
    if (sht->height >= 0)
    { /* 如果正在显示，则按新图层的信息刷新画面 */
        sheet_refreshsub(old_vx0, old_vy0, old_vx0 + sht->bxsize,
                         old_vy0 + sht->bysize);
        sheet_refreshsub(vx0, vy0, vx0 + sht->bxsize, vy0 + sht->bysize);
    }
}

void sheet_free(SHEET *sht)
{
    if (sht->height >= 0)
    {
        sheet_updown(sht, -1); /* 如果处于显示状态，则先设定为隐藏 */
    }
    sht->flags = 0; /* "未使用"标志 */
}