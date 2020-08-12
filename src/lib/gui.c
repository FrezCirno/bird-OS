#include <bird/memory.h>
#include <bird/gui.h>
#include <glib.h>
#include <stddef.h>

WINDOW windows[MAX_WINDOW_COUNT];

WINDOW *createWindow(int x, int y, int xsize, int ysize, const char *title,
                     int flags)
{
    for (int i = 0; i < MAX_WINDOW_COUNT; i++)
    {
        WINDOW *win = windows + i;
        if (!(win->flags & WND_INUSE))
        {
            win->flags |= WND_INUSE;
            win->sht = alloc_sheet();
            sheet_setbuf(win->sht, (unsigned char *)mm_alloc_4k(xsize * ysize),
                         xsize, ysize, -1);
            drawWindowTo(win->sht->buf, xsize, xsize, ysize, title);
            movexy(win->sht, x, y);
            movez(win->sht, ctl->top);
            // 内部使用, 表示窗口去掉边框和标题栏的部分
            win->body.buf = &win->sht->buf[21 * win->sht->bxsize + 3];
            win->body.vx0 += 3;
            win->body.bxsize = win->sht->bxsize; // pitch 不变
            win->body.vy0 += 23;
            win->body.bysize -= 23;
            return win;
        }
    }
    return NULL;
}