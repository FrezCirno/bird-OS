#pragma once
#include <types.h>

extern int ticks;

void init_clock();
void clock_handler(u32 irq);