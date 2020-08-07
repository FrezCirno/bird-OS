#pragma once
#include <stdarg.h>

int vsprintf(char *buf, const char *fmt, va_list args);
int printf(const char *fmt, ...);