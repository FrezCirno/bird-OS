#pragma once
#include <glib.h>

#define MAX_WINDOW_COUNT 100

typedef struct s_window
{
    int x, y;
    SHEET *sht, body;
    unsigned char *content;
    int flags;
} WINDOW;

#define WND_INUSE   1
#define WND_MINIMUM 2

WINDOW *createWindow(int x, int y, int xsize, int ysize, const char *title,
                     int flags);