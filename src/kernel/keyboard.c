#include <io.h>       // io_cli, io_sti
#include <int.h>      // put_irq_handler
#include <keyboard.h> // KB_INPUT
#include <keymap.h>
#include <glib.h>   // printstr
#include <string.h> // itoa
#include <buffer.h> // buffer

u8 kb_buf[KB_SIZE];
FIFO_BUFFER kb_in;

int code_with_E0 = 0;
int shift_l;     /* l shift state */
int shift_r;     /* r shift state */
int alt_l;       /* l alt state	 */
int alt_r;       /* r left state	 */
int ctrl_l;      /* l ctrl state	 */
int ctrl_r;      /* l ctrl state	 */
int caps_lock;   /* Caps Lock	 */
int num_lock;    /* Num Lock	 */
int scroll_lock; /* Scroll Lock	 */
int column;

void init_keyboard()
{
    fifo_init(&kb_in, KB_SIZE, kb_buf);
    put_irq_handler(INT_VECTOR_IRQ_KEYBOARD, keyboard_handler);
    enable_irq(INT_VECTOR_IRQ_KEYBOARD);
}

void keyboard_handler(u32 irq)
{
    fifo_push(&kb_in, in8(KB_DATA));
}

void keyboard_read()
{
    char output[2];

    u32 key = 0; /* 用一个整型来表示一个键。比如，如果 Home 被按下，
                  * 则 key 值将为定义在 keyboard.h 中的 'HOME'。
                  */

    if (kb_in.size)
    {
        io_cli();
        u8 scan_code = fifo_pop(&kb_in);
        io_sti();

        /* 下面开始解析扫描码 */
        if (scan_code == 0xE1)
        {
            /* 暂时不做任何操作 */
        }
        else if (scan_code == 0xE0)
        {
            code_with_E0 = 1;
        }
        else
        { /* 下面处理可打印字符 */

            /* 首先判断Make Code 还是 Break Code */
            int is_make = (scan_code & FLAG_BREAK ? 0 : 1);

            /* 先定位到 keymap 中的行 */
            u32 *keyrow = keymap[scan_code & 0x7F];

            column = 0;
            if (shift_l || shift_r)
            {
                column = 1;
            }
            if (code_with_E0)
            {
                column       = 2;
                code_with_E0 = 0;
            }

            key = keyrow[column];

            switch (key)
            {
            case SHIFT_L:
                shift_l = is_make;
                key     = 0;
                break;
            case SHIFT_R:
                shift_r = is_make;
                key     = 0;
                break;
            case CTRL_L:
                ctrl_l = is_make;
                key    = 0;
                break;
            case CTRL_R:
                ctrl_r = is_make;
                key    = 0;
                break;
            case ALT_L:
                alt_l = is_make;
                key   = 0;
                break;
            case ALT_R:
                alt_l = is_make;
                key   = 0;
                break;
            default:
                if (!is_make)
                {            /* 如果是 Break Code */
                    key = 0; /* 忽略之 */
                }
                break;
            }

            /* 如果 Key 不为0说明是可打印字符，否则不做处理 */
            if (key)
            {
                output[0] = key;
                putchar(output[0], PEN_RED);
            }
        }
    }
}
