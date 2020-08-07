#include <asm/io.h>
#include <bird/memory.h>
#include <glib.h>
#include <string.h>
#include <font.h>

unsigned char *vram = (unsigned char *)0xa0000;
int scr_x;
int scr_y;
int scr_bpp;
int scr_pitch;

int cur_x;
int cur_y;

int fontsmap[65536];

const unsigned char cursor[16] = {
    X_______, XX______, XXX_____, XXXX____, XXXXX___, XXXXXX__, XXXXXXX_,
    XXXXXXXX, XXXXX___, XX_XX___, ____XX__, ____XX__, _____XX_};

const unsigned char closebtn[2][16] = {
    {________, ________, ________, ____XX__, _____XX_, ______XX, _______X,
     ______XX, _____XX_, ____XX__, ________, ________, ________},
    {________, ________, ________, __XX____, _XX_____, XX______, X_______,
     XX______, _XX_____, __XX____, ________, ________, ________}};

#define abs(x) ((x) > 0 ? (x) : (-(x)))

typedef struct s_shtctl
{
    unsigned char *map; // 大小等于xsize*ysize, 用来表示每个像素属于哪个图层
    int xsize, ysize, top;
    SHEET *sheets[MAX_SHEETS];
    SHEET _sheets[MAX_SHEETS];
} SHTCTL;

SHTCTL *ctl;

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

    init_sheets(scr_x, scr_y);

    // 默认颜色背景
    SHEET *bg = alloc_sheet();
    sheet_setbuf(bg, (unsigned char *)mm_alloc_4k(scr_x * scr_y), scr_x, scr_y,
                 -1);
    memset(bg->buf, PEN_BLACK, scr_x * scr_y);
    movez(bg, 0);
}

void initPalette()
{
    static const unsigned char rgb[48] = {
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

void setPalette(int start, int end, const unsigned char *palette)
{
    unsigned int eflags = io_load_eflags();
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
    for (unsigned int i = 0; i < fonts.Chars; i++)
    {
        unsigned short index = fonts.Index[i];
        fontsmap[index]      = i;
    }
}

void putPixelTo(unsigned char *dst, int pitch, int x, int y, int color)
{
    unsigned int base = y * pitch + x;
    dst[base]         = color;
}

void putPixel(int x, int y, int color)
{
    // 每个bank 0xa0000-0xb0000 共 0x10000 = 64K 字节
    // 显存一共800*600共 480000 字节
    // 8个bank
    // bank n offset m <-> (n << 16) | m <-> 0xa0000 | m
    unsigned int base = y * scr_pitch + x;
    if (base < bank_start || base >= bank_end)
    {
        bga_set_bank(base >> 16);
    }
    vram[base & 0xffff] = color;
}

void drawLineTo(unsigned char *dst, int pitch, int x0, int y0, int x1, int y1,
                int color)
{
    // Bresenhamline算法
    int dx  = x1 - x0;           // x偏移量
    int dy  = y1 - y0;           // y偏移量
    int ux  = (dx > 0 ? 1 : -1); // x伸展方向
    int uy  = (dy > 0 ? 1 : -1); // y伸展方向
    int dx2 = dx << 1;           // x偏移量乘2
    int dy2 = dy << 1;           // y偏移量乘2
    if (abs(dx) > abs(dy))
    {                //以x为增量方向计算
        int e = -dx; // e = -0.5 * 2 * dx,把e 用2 * dx* e替换
        int y = y0;  //起点y坐标
        for (int x = x0; x < x1; x += ux) //起点x坐标
        {
            putPixelTo(dst, pitch, x, y, color);
            e = e + dy2; //来自 2*e*dx= 2*e*dx + 2dy  （原来是 e = e + k）
            if (e > 0) // e是整数且大于0时表示要取右上的点（否则是右下的点）
            {
                y += uy;
                e = e - dx2; // 2*e*dx = 2*e*dx - 2*dx  (原来是 e = e -1)
            }
        }
    }
    else
    {                //以y为增量方向计算
        int e = -dy; // e = -0.5 * 2 * dy,把e 用2 * dy* e替换
        int x = x0;  //起点x坐标
        for (int y = y0; y < y1; y += uy) //起点y坐标
        {
            putPixelTo(dst, pitch, x, y, color);
            e = e + dx2; //来自 2*e*dy= 2*e*dy + 2dy  （原来是 e = e + k）
            if (e > 0) // e是整数且大于0时表示要取右上的点（否则是右下的点）
            {
                x += ux;
                e = e - dy2; // 2*e*dy = 2*e*dy - 2*dy  (原来是 e = e -1)
            }
        }
    }
}

void drawLine(int x0, int y0, int x1, int y1, int color)
{
    // Bresenhamline算法
    int dx  = x1 - x0;           // x偏移量
    int dy  = y1 - y0;           // y偏移量
    int ux  = (dx > 0 ? 1 : -1); // x伸展方向
    int uy  = (dy > 0 ? 1 : -1); // y伸展方向
    int dx2 = dx << 1;           // x偏移量乘2
    int dy2 = dy << 1;           // y偏移量乘2
    if (abs(dx) > abs(dy))
    {                //以x为增量方向计算
        int e = -dx; // e = -0.5 * 2 * dx,把e 用2 * dx* e替换
        int y = y0;  //起点y坐标
        for (int x = x0; x < x1; x += ux) //起点x坐标
        {
            putPixel(x, y, color);
            e = e + dy2; //来自 2*e*dx= 2*e*dx + 2dy  （原来是 e = e + k）
            if (e > 0) // e是整数且大于0时表示要取右上的点（否则是右下的点）
            {
                y += uy;
                e = e - dx2; // 2*e*dx = 2*e*dx - 2*dx  (原来是 e = e -1)
            }
        }
    }
    else
    {                //以y为增量方向计算
        int e = -dy; // e = -0.5 * 2 * dy,把e 用2 * dy* e替换
        int x = x0;  //起点x坐标
        for (int y = y0; y < y1; y += uy) //起点y坐标
        {
            putPixel(x, y, color);
            e = e + dx2; //来自 2*e*dy= 2*e*dy + 2dy  （原来是 e = e + k）
            if (e > 0) // e是整数且大于0时表示要取右上的点（否则是右下的点）
            {
                x += ux;
                e = e - dy2; // 2*e*dy = 2*e*dy - 2*dy  (原来是 e = e -1)
            }
        }
    }
}

void drawRectTo(unsigned char *dst, int pitch, int x1, int y1, int x2, int y2,
                int color)
{
    for (int y = y1; y < y2; y++)
    {
        putPixelTo(dst, pitch, x1, y, color);
        putPixelTo(dst, pitch, x2, y, color);
    }
    for (int x = x1; x < x2; x++)
    {
        putPixelTo(dst, pitch, x, y1, color);
        putPixelTo(dst, pitch, x, y2, color);
    }
}

void drawRect(int x1, int y1, int x2, int y2, int color)
{
    for (int y = y1; y < y2; y++)
    {
        putPixel(x1, y, color);
        putPixel(x2, y, color);
    }
    for (int x = x1; x < x2; x++)
    {
        putPixel(x, y1, color);
        putPixel(x, y2, color);
    }
}

void fillRectTo(unsigned char *dst, int pitch, int x1, int y1, int x2, int y2,
                int color)
{
    for (int y = y1; y < y2; y++)
    {
        for (int x = x1; x < x2; x++)
        {
            putPixelTo(dst, pitch, x, y, color);
        }
    }
}

void fillRect(int x1, int y1, int x2, int y2, int color)
{
    for (int y = y1; y < y2; y++)
    {
        for (int x = x1; x < x2; x++)
        {
            putPixel(x, y, color);
        }
    }
}

void drawGlyphTo(unsigned char *dst, int pitch, int x, int y,
                 const unsigned char *glyph, int color)
{
    unsigned int base = y * pitch + x;
    for (int in_y = 0; in_y < 16; in_y++)
    {
        unsigned char line   = glyph[in_y];
        unsigned char mask   = 0x80;
        unsigned int in_base = base;
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

void drawGlyph(int x, int y, const unsigned char *glyph, int color)
{
    unsigned int base = y * scr_pitch + x;
    for (int in_y = 0; in_y < fonts.Height; in_y++)
    {
        unsigned char line   = glyph[in_y];
        unsigned char mask   = 0x80;
        unsigned int in_base = base;
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

void drawCharTo(unsigned char *dst, int pitch, int x, int y, char ch, int color)
{
    const unsigned char *font_data = &fonts.Bitmap[16 * fontsmap[ch]];
    drawGlyphTo(dst, pitch, x, y, font_data, color);
}

void drawChar(int x, int y, char ch, int color)
{
    const unsigned char *font_data = &fonts.Bitmap[16 * fontsmap[ch]];
    drawGlyph(x, y, font_data, color);
}

void drawTextTo(unsigned char *dst, int pitch, int x, int y, const char *str,
                int color)
{
    while (*str)
    {
        drawCharTo(dst, pitch, x, y, *str++, color);
        x += 8;
    }
}

void drawText(int x, int y, const char *str, int color)
{
    while (*str)
    {
        drawChar(x, y, *str++, color);
        x += 8;
    }
}

void gotoxy(int x, int y)
{
    cur_x = x;
    cur_y = y;
}

void putchar(char ch, int color)
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

void printstr(const char *str, int color)
{
    while (*str)
    {
        putchar(*str++, color);
    }
}

void drawWindowTo(unsigned char *buf, int pitch, int xsize, int ysize,
                  const char *title)
{
    drawLineTo(buf, pitch, 0, 0, xsize, 0, PEN_LIGHT_GRAY);            // top
    drawLineTo(buf, pitch, 1, 1, xsize - 1, 1, PEN_WHITE);             // top2
    drawLineTo(buf, pitch, 0, 0, 0, ysize, PEN_LIGHT_GRAY);            // left
    drawLineTo(buf, pitch, 1, 1, 1, ysize - 1, PEN_WHITE);             // left2
    drawLineTo(buf, pitch, xsize - 1, 0, xsize - 1, ysize, PEN_BLACK); // right
    drawLineTo(buf, pitch, xsize - 2, 1, xsize - 2, ysize - 1,
               PEN_DARK_GRAY); // right2
    drawLineTo(buf, pitch, 0, ysize - 1, xsize, ysize - 1,
               PEN_BLACK); // bottom
    drawLineTo(buf, pitch, 1, ysize - 2, xsize - 1, ysize - 2,
               PEN_DARK_GRAY); // bottom2
    fillRectTo(buf, pitch, 2, 2, xsize - 2, ysize - 2, PEN_LIGHT_GRAY); // body
    fillRectTo(buf, pitch, 3, 3, xsize - 3, 20, PEN_DARK_CLAN); // header
    drawTextTo(buf, pitch, 24, 4, title, PEN_WHITE);

    fillRectTo(buf, pitch, xsize - 19, 5, xsize - 5, 18, PEN_LIGHT_GRAY);
    drawGlyphTo(buf, pitch, xsize - 20, 5, closebtn[0], PEN_BLACK);
    drawGlyphTo(buf, pitch, xsize - 20 + 8, 5, closebtn[1], PEN_BLACK);
}

int init_sheets(int x_size, int y_size)
{
    ctl = (SHTCTL *)mm_alloc_4k(sizeof(SHTCTL));
    if (ctl == 0)
    {
        return -1;
    }
    ctl->map = (unsigned char *)mm_alloc_4k(x_size * y_size);
    if (ctl->map == 0)
    {
        mm_free_4k((unsigned int)ctl, sizeof(MEMMAN));
        return -1;
    }
    ctl->xsize = x_size;
    ctl->ysize = y_size;
    ctl->top   = -1;
    for (int i = 0; i < MAX_SHEETS; i++)
    {
        ctl->_sheets[i].flags = 0;
    }
    return 0;
}

SHEET *alloc_sheet()
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

void free_sheet(SHEET *sht)
{
    if (sht->height >= 0)
    {
        movez(sht, -1); /* 如果处于显示状态，则先设定为隐藏 */
    }
    sht->flags = 0; /* "未使用"标志 */
}

void sheet_setbuf(SHEET *sht, unsigned char *buf, int xsize, int ysize,
                  int col_inv)
{
    sht->buf     = buf;
    sht->bxsize  = xsize;
    sht->bysize  = ysize;
    sht->col_inv = col_inv;
}

void movez(SHEET *sht, int height)
{
    int old = sht->height;
    // 边界处理
    if (height > ctl->top + 1) height = ctl->top + 1;
    if (height < -1) height = -1; // -1隐藏
    sht->height = height;         // 设定高度

    /* 下面主要是进行sheets[ ]的重新排列 */
    if (old > height)
    { /* 比以前低 */
        if (height >= 0)
        {
            /* 把中间的往上提 */
            for (int h = old; h > height; h--)
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
                for (int h = old; h < ctl->top; h++)
                {
                    ctl->sheets[h]         = ctl->sheets[h + 1];
                    ctl->sheets[h]->height = h;
                }
            }
            ctl->top--; /* 由于显示中的图层减少了一个，所以最上面的图层高度下降 */
        }
        /* 按新图层的信息重新绘制画面 */
        refresh_map(sht->vx0, sht->vy0, sht->vx0 + sht->bxsize,
                    sht->vy0 + sht->bysize, height);
        refresh(sht->vx0, sht->vy0, sht->vx0 + sht->bxsize,
                sht->vy0 + sht->bysize, height, height);
    }
    else if (old < height)
    { /* 比以前高 */
        if (old >= 0)
        {
            /* 把中间的拉下去 */
            for (int h = old; h < height; h++)
            {
                ctl->sheets[h]         = ctl->sheets[h + 1];
                ctl->sheets[h]->height = h;
            }
            ctl->sheets[height] = sht;
        }
        else
        { /* 由隐藏状态转为显示状态 */
            /* 将已在上面的提上来 */
            for (int h = ctl->top; h >= height; h--)
            {
                ctl->sheets[h + 1]         = ctl->sheets[h];
                ctl->sheets[h + 1]->height = h + 1;
            }
            ctl->sheets[height] = sht;
            ctl->top++; /* 由于已显示的图层增加了1个，所以最上面的图层高度增加 */
        }
        /* 按新图层信息重新绘制画面 */
        refresh_map(sht->vx0, sht->vy0, sht->vx0 + sht->bxsize,
                    sht->vy0 + sht->bysize, height);
        refresh(sht->vx0, sht->vy0, sht->vx0 + sht->bxsize,
                sht->vy0 + sht->bysize, height, height);
    }
}

void refresh_local(SHEET *sht, int bx0, int by0, int bx1, int by1)
{
    if (sht->height >= 0)
    { /* 如果正在显示，则按新图层的信息刷新画面*/
        refresh(sht->vx0 + bx0, sht->vy0 + by0, sht->vx0 + bx1, sht->vy0 + by1,
                sht->height, sht->height);
    }
}

void refresh_map(int vx0, int vy0, int vx1, int vy1, int h0)
{
    /* 如果refresh的范围超出了画面则修正 */
    if (vx0 < 0) vx0 = 0;
    if (vy0 < 0) vy0 = 0;
    if (vx1 > ctl->xsize) vx1 = ctl->xsize;
    if (vy1 > ctl->ysize) vy1 = ctl->ysize;

    for (int h = h0; h <= ctl->top; h++)
    {
        SHEET *sht         = ctl->sheets[h];
        int sid            = (int)(sht - ctl->_sheets);
        unsigned char *buf = sht->buf;
        unsigned char *map = ctl->map;

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
                int vx              = sht->vx0 + bx;
                unsigned char color = buf[by * sht->bxsize + bx];
                int voffset         = vy * ctl->xsize + vx;
                if (color != sht->col_inv)
                {
                    map[voffset] = sid;
                }
            }
        }
    }
}

void refresh(int vx0, int vy0, int vx1, int vy1, int h0, int h1)
{
    /* 如果refresh的范围超出了画面则修正 */
    if (vx0 < 0) vx0 = 0;
    if (vy0 < 0) vy0 = 0;
    if (vx1 > ctl->xsize) vx1 = ctl->xsize;
    if (vy1 > ctl->ysize) vy1 = ctl->ysize;
    if (h1 > ctl->top) h1 = ctl->top;

    for (int h = h0; h <= h1; h++)
    {
        SHEET *sht         = ctl->sheets[h];
        int sid            = (int)(sht - ctl->_sheets);
        unsigned char *buf = sht->buf;
        unsigned char *map = ctl->map;

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
                int vx      = sht->vx0 + bx;
                int voffset = vy * ctl->xsize + vx;
                if (map[voffset] == sid)
                {
                    unsigned char color = buf[by * sht->bxsize + bx];
                    putPixel(vx, vy, color);
                }
            }
        }
    }
}

void movexy(SHEET *sht, int vx0, int vy0)
{
    int old_vx0 = sht->vx0, old_vy0 = sht->vy0;
    sht->vx0 = vx0;
    sht->vy0 = vy0;
    if (sht->height >= 0)
    { /* 如果正在显示，则按新图层的信息刷新画面 */
        refresh_map(old_vx0, old_vy0, old_vx0 + sht->bxsize,
                    old_vy0 + sht->bysize, 0);
        refresh_map(vx0, vy0, vx0 + sht->bxsize, vy0 + sht->bysize, sht->height);
        refresh(old_vx0, old_vy0, old_vx0 + sht->bxsize, old_vy0 + sht->bysize,
                0, sht->height - 1);
        refresh(vx0, vy0, vx0 + sht->bxsize, vy0 + sht->bysize, sht->height,
                sht->height);
    }
}
