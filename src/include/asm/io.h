#pragma once

void io_hlt();

unsigned int io_load_eflags();
void io_store_eflags(unsigned int eflags);

unsigned int io_load_cr0();
void io_store_cr0(unsigned int cr0);

unsigned int io_load_cr1();
void io_store_cr1(unsigned int cr1);

unsigned int io_load_cr2();
void io_store_cr2(unsigned int cr0);

unsigned int io_load_cr3();
void io_store_cr3(unsigned int cr0);

void io_sti();
void io_cli();

unsigned char in8(unsigned int port);
unsigned short in16(unsigned int port);
unsigned int in32(unsigned int port);

void out8(unsigned int port, unsigned char value);
void out16(unsigned int port, unsigned short value);
void out32(unsigned int port, unsigned int value);