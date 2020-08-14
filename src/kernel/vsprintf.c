#include <stdarg.h>
#include <string.h>
#include <ctype.h>

#define STR_DEFAULT_LEN 1024

int vsprintf(char *buf, const char *fmt, va_list ap)
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
            *qbuf++ = va_arg(ap, char);
            break;
        case 'x': // %0?\d*x
            m   = va_arg(ap, int);
            tmp = itoa(m, 16);
            strcpy(qbuf, tmp);
            qbuf += strlen(tmp);
            break;
        case 'd': // %0?\d*d
            m   = va_arg(ap, int);
            tmp = itoa(m, 10);
            strcpy(qbuf, tmp);
            qbuf += strlen(tmp);
            break;
        case 's': // %0?\d*s
            tmp = va_arg(ap, char *);
            strcpy(qbuf, tmp);
            qbuf += strlen(tmp);
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
    int ret;
    va_list ap;
    va_start(ap, fmt);
    ret = vsprintf(buf, fmt, ap);
    va_end(ap);
    return ret;
}