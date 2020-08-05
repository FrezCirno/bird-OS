#include <types.h>
#include <io.h>
#include <BGA.h>

u32 cur_bank;
u32 bank_start, bank_end;
const u32 bank_size = 0x10000;

void bga_write_register(u16 index, u16 data)
{
    out16(VBE_DISPI_IOPORT_INDEX, index);
    out16(VBE_DISPI_IOPORT_DATA, data);
}

u16 bga_read_register(u16 index)
{
    out16(VBE_DISPI_IOPORT_INDEX, index);
    return in16(VBE_DISPI_IOPORT_DATA);
}

void bga_set_video_mode(u32 Width, u32 Height, u32 BitDepth, int use_lfb,
                        int clear)
{
    bga_write_register(VBE_DISPI_INDEX_ENABLE, VBE_DISPI_DISABLED);
    bga_write_register(VBE_DISPI_INDEX_XRES, Width);
    bga_write_register(VBE_DISPI_INDEX_YRES, Height);
    bga_write_register(VBE_DISPI_INDEX_BPP, BitDepth);
    bga_write_register(VBE_DISPI_INDEX_ENABLE,
                       VBE_DISPI_ENABLED | (use_lfb ? VBE_DISPI_LFB_ENABLED : 0)
                           | (clear ? 0 : VBE_DISPI_NOCLEARMEM));
}

void bga_set_bank(u16 BankNumber)
{
    cur_bank   = BankNumber;
    bank_start = cur_bank * bank_size;
    bank_end   = bank_start + bank_size;
    bga_write_register(VBE_DISPI_INDEX_BANK, BankNumber);
}