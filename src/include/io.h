#pragma once
#include <types.h>

void io_hlt();

u32 io_load_eflags();
void io_store_eflags(u32 eflags);

u32 io_load_cr0();
void io_store_cr0(u32 cr0);

void io_sti();
void io_cli();

u8 in8(u32 port);
u16 in16(u32 port);
u32 in32(u32 port);

void out8(u32 port, u8 value);
void out16(u32 port, u16 value);
void out32(u32 port, u32 value);