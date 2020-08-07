#pragma once

extern int ticks;

void init_clock();

void clock_handler(unsigned int irq);

int sys_getticks();