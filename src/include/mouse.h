#pragma once

#define MOUSE_PORT_DATA     0x0060
#define MOUSE_PORT_CMD      0x0064
#define MOUSE_NOT_READY     0x02
#define MOUSE_WRITE_MODE    0x60
#define KBC_MODE            0x47
#define KEYCMD_SENDTO_MOUSE 0xd4
#define MOUSE_ENABLE        0xf4

typedef struct s_mouse_dec
{
    unsigned char buf[3], phase;
    int x, y, btn;
} MOUSE_DEC;

extern int mouse_x, mouse_y;

void wait_KBC_sendready();
void init_mouse();
void mouse_handler(unsigned int irq);
void mouse_read();