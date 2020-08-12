#include <stdarg.h>
#include <string.h>
#include <ctype.h>

#define STR_DEFAULT_LEN 1024

int vsprintf(char *buf, const char *fmt, va_list args)
{
    char *pbuf;

    va_list p_next_arg = args;
    int m;

    char inner_buf[STR_DEFAULT_LEN];
    char cs;
    int align_nr;

    for (pbuf = buf; *fmt; fmt++)
    {
        if (*fmt != '%')
        {
            *pbuf++ = *fmt;
            continue;
        }

        /* a format string begins */
        align_nr = 0;

        fmt++;

        if (*fmt == '%')
        {
            *pbuf++ = *fmt;
            continue;
        }

        if (*fmt == '0')
        {
            cs = '0';
            fmt++;
        }
        else
        {
            cs = ' ';
        }

        while (isdigit(*fmt))
        {
            align_nr *= 10;
            align_nr += *fmt - '0';
            fmt++;
        }

        char *qbuf = inner_buf;
        memset(qbuf, 0, sizeof(inner_buf));

        switch (*fmt)
        {
        case 'c':
            *qbuf++ = *((char *)p_next_arg);
            p_next_arg += 4;
            break;
        case 'x':
            m    = *((int *)p_next_arg);
            qbuf = strcpy(qbuf, itoa(m, 16));
            p_next_arg += 4;
            break;
        case 'd':
            m    = *((int *)p_next_arg);
            qbuf = strcpy(qbuf, itoa(m, 10));
            p_next_arg += 4;
            break;
        case 's':
            strcpy(qbuf, (*((char **)p_next_arg)));
            qbuf += strlen(*((char **)p_next_arg));
            p_next_arg += 4;
            break;
        default: break;
        }

        for (int l = strlen(inner_buf), k = ((align_nr > l) ? (align_nr - l) : 0);
             k > 0; k--)
        {
            *pbuf++ = cs;
        }
        pbuf = strcpy(pbuf, inner_buf);
    }
    *pbuf = 0;
    return (pbuf - buf);
}

int sprintf(char *buf, const char *fmt, ...)
{
    va_list arg =
        (va_list)((char *)(&fmt) + 4); /* 4 是参数 fmt 所占堆栈中的大小 */
    return vsprintf(buf, fmt, arg);
}