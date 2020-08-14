#include <stdarg.h>
#include <string.h>
#include <ctype.h>

#define STR_DEFAULT_LEN 1024

int vsprintf(char *buf, const char *fmt, va_list args)
{
    char *pbuf = buf;
    int m;
    const char *tmp;

    char inner_buf[STR_DEFAULT_LEN];

    for (; *fmt; fmt++)
    {
        /* (%) */
        if (*fmt != '%')
        {
            *pbuf++ = *fmt;
            continue;
        }

        // %.*
        if (*++fmt == '%') // %%
        {
            *pbuf++ = *fmt;
            continue;
        }

        // %(0?)
        char cs = (*fmt == '0' ? '0' : ' ');
        if (cs == '0') ++fmt;

        int align_nr = 0;
        while (isdigit(*fmt)) // %0?(\d*)
        {
            align_nr *= 10;
            align_nr += *fmt++ - '0';
        }

        // %0?\d*([^\d]*)
        char *qbuf = inner_buf;

        switch (*fmt)
        {
        case 'c': // %0?\d*c
            *qbuf++ = *(char *)args;
            args += 4;
            break;
        case 'x': // %0?\d*x
            m   = *(int *)args;
            tmp = itoa(m, 16);
            strcpy(qbuf, tmp);
            qbuf += strlen(tmp);
            args += 4;
            break;
        case 'd': // %0?\d*d
            m   = *(int *)args;
            tmp = itoa(m, 10);
            strcpy(qbuf, tmp);
            qbuf += strlen(tmp);
            args += 4;
            break;
        case 's': // %0?\d*s
            strcpy(qbuf, *(char **)args);
            qbuf += strlen(*(char **)args);
            args += 4;
            break;
        default: break; // %0?\d*[^cxds] 不处理
        }
        // #0?\d*\w
        for (int l = strlen(inner_buf), k = ((align_nr > l) ? (align_nr - l) : 0);
             k > 0; k--)
            *pbuf++ = cs;
        strcpy(pbuf, inner_buf);
        pbuf += strlen(inner_buf);
    }
    *pbuf = '\0';
    return (pbuf - buf);
}

int sprintf(char *buf, const char *fmt, ...)
{
    va_list arg =
        (va_list)((char *)(&fmt) + 4); /* 4 是参数 fmt 所占堆栈中的大小 */
    return vsprintf(buf, fmt, arg);
}