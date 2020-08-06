#pragma once
#include <types.h>
#include <font.h>
#include <BGA.h>

#define MAX_SHEETS   256
#define SHEET_IN_USE 1

typedef struct s_sheet
{
    u8 *buf;
    int bxsize, bysize; // 图层大小
    int vx0, vy0;       // 图层位置
    int col_inv;        // 透明色色号
    int height;         // 图层高度
    int flags;          // 图层设定
} SHEET;

typedef struct s_shtctl
{
    int *map; // 大小等于xsize*ysize, 用来表示每个像素属于哪个图层
    int xsize, ysize, top;
    SHEET *sheets[MAX_SHEETS];
    SHEET _sheets[MAX_SHEETS];
} SHTCTL;

extern u32 scr_x;
extern u32 scr_y;
extern u32 scr_bpp;

#define PEN_BLACK        0  //黑
#define PEN_RED          1  //梁红
#define PEN_LIGHT_GREEN  2  //亮绿
#define PEN_LIGHT_YELLOW 3  //亮黄
#define PEN_LIGHT_BLUE   4  //亮蓝
#define PEN_LIGHT_PURPLE 5  //亮紫
#define PEN_BLUE         6  //浅亮蓝
#define PEN_WHITE        7  //白
#define PEN_LIGHT_GRAY   8  //亮灰
#define PEN_DARK_RED     9  //暗红
#define PEN_DARK_GREEN   10 //暗绿
#define PEN_DARK_YELLOW  11 //暗黄
#define PEN_DARK_CLAN    12 //暗青
#define PEN_DARK_PURPLE  13 //暗紫
#define PEN_DARK_BLUE    14 //浅暗蓝
#define PEN_DARK_GRAY    15 //暗灰

extern const u8 cursor[16];

void initPalette();

void cacheFonts();

void init_video();

void setPalette(int start, int end, const u8 *palette);

void putPixelTo(u8 *dst, int pitch, int x, int y, u8 color);

void putPixel(int x, int y, u8 color);

void drawRectTo(u8 *dst, int pitch, int x1, int y1, int x2, int y2, u8 color);

void drawRect(int x1, int y1, int x2, int y2, u8 color);

void fillRectTo(u8 *dst, int pitch, int x1, int y1, int x2, int y2, u8 color);

void fillRect(int x1, int y1, int x2, int y2, u8 color);

void drawLineTo(u8 *dst, int pitch, int x0, int y0, int x1, int y1, u8 color);

void drawLine(int x0, int y0, int x1, int y1, u8 color);

void drawGlyphTo(u8 *dst, int pitch, int x, int y, const u8 *glyph, u8 color);

void drawGlyph(int x, int y, const u8 *glyph, u8 color);

void drawCharTo(u8 *dst, int pitch, int x, int y, char ch, u8 color);

void drawChar(int x, int y, char ch, u8 color);

void drawTextTo(u8 *dst, int pitch, int x, int y, const char *str, u8 color);

void drawText(int x, int y, const char *str, u8 color);

// 依次输出
void gotoxy(u32 x, u32 y);

void putchar(char ch, u8 color);

void printstr(const char *str, u8 color);

// 图层控制, 在init_video中调用
int shtctl_init(int x_size, int y_size);

// 分配新的图层
SHEET *sheet_alloc();

// 设定图层信息, height==-1表示隐藏
void sheet_setbuf(SHEET *sht, u8 *buf, int xsize, int ysize, int col_inv);

// 图层水平移动
void sheet_slide(SHEET *sht, int vx0, int vy0);

// 设置图层高度
void sheet_updown(SHEET *sht, int height);

// 刷新图层sht内部clipbox的屏幕区域
void sheet_refresh_sheet(SHEET *sht, int bx0, int by0, int bx1, int by1);

// 刷新屏幕clipbox区域的所有图层
void sheet_refresh(int vx0, int vy0, int vx1, int vy1, int h0, int h1);

// 释放图层
void sheet_free(SHEET *sht);

void drawWindowTo(u8 *buf, int pitch, int xsize, int ysize, char *title);