#pragma once
#include <stdarg.h>

/* the assert macro */
#define ASSERT
#ifdef ASSERT
void assertion_failure(char *exp, char *file, char *base_file, int line);
    #define assert(exp) \
        if (!(exp)) assertion_failure(#exp, __FILE__, __BASE_FILE__, __LINE__)
#else
    #define assert(exp)
#endif

/* string */

#define O_CREAT 1
#define O_RDWR  2
#define O_TRUNC 4

#define SEEK_SET 1
#define SEEK_CUR 2
#define SEEK_END 3

#define MAX_PATH 128

/**
 * @struct stat
 * @brief  File status, returned by syscall stat();
 */
struct stat
{
    int st_dev;  /* major/minor device number */
    int st_ino;  /* i-node number */
    int st_mode; /* file mode, protection bits, etc. */
    int st_rdev; /* device ID (if special file) */
    int st_size; /* file size */
};

/**
 * @struct time
 * @brief  RTC time from CMOS.
 */
struct time
{
    unsigned char year;
    unsigned char month;
    unsigned char day;
    unsigned char hour;
    unsigned char minute;
    unsigned char second;
};

#define BCD_TO_DEC(x) ((x >> 4) * 10 + (x & 0x0f))

/* printf.c */
int printf(const char *fmt, ...);

/* vsprintf.c */
int vsprintf(char *buf, const char *fmt, va_list args);
int sprintf(char *buf, const char *fmt, ...);

/*--------*/
/* 库函数 */
/*--------*/

#ifdef ENABLE_DISK_LOG
    #define SYSLOG syslog
#endif

/* lib/open.c */
int open(const char *pathname, int flags);

/* lib/close.c */
int close(int fd);

/* lib/read.c */
int read(int fd, void *buf, int count);

/* lib/write.c */
int write(int fd, const void *buf, int count);

/* lib/lseek.c */
int lseek(int fd, int offset, int whence);

/* lib/unlink.c */
int unlink(const char *pathname);

/* lib/getpid.c */
int getpid();

/* lib/fork.c */
int fork();

/* lib/exit.c */
void exit(int status);

/* lib/wait.c */
int wait(int *status);

/* lib/exec.c */
int exec(const char *path);
int execl(const char *path, const char *arg, ...);
int execv(const char *path, char *argv[]);

/* lib/stat.c */
int stat(const char *path, struct stat *buf);

/* lib/syslog.c */
int syslog(const char *fmt, ...);
