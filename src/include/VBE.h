#pragma once
#include <types.h>

struct ModeInfoBlock
{
    u16 attributes;
    u8 winA, winB;
    u16 granularity;
    u16 winsize;
    u16 segmentA, segmentB;
    VBE_FAR(realFctPtr);
    u16 pitch; // bytes per scanline

    u16 Xres, Yres;
    u8 Wchar, Ychar, planes, bpp, banks;
    u8 memory_model, bank_size, image_pages;
    u8 reserved0;

    u8 red_mask, red_position;
    u8 green_mask, green_position;
    u8 blue_mask, blue_position;
    u8 rsv_mask, rsv_position;
    u8 directcolor_attributes;

    u32 physbase; // your LFB (Linear Framebuffer) address ;)
    u32 reserved1;
    u16 reserved2;
} __attribute__((packed));