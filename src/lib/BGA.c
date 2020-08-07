#include <asm/io.h>
#include <BGA.h>

unsigned int cur_bank;
unsigned int bank_start, bank_end;
const unsigned int bank_size = 0x10000;

void bga_write_register(unsigned short index, unsigned short data)
{
    out16(VBE_DISPI_IOPORT_INDEX, index);
    out16(VBE_DISPI_IOPORT_DATA, data);
}

unsigned short bga_read_register(unsigned short index)
{
    out16(VBE_DISPI_IOPORT_INDEX, index);
    return in16(VBE_DISPI_IOPORT_DATA);
}

void bga_set_video_mode(unsigned int Width, unsigned int Height,
                        unsigned int BitDepth, int use_lfb, int clear)
{
    bga_write_register(VBE_DISPI_INDEX_ENABLE, VBE_DISPI_DISABLED);
    bga_write_register(VBE_DISPI_INDEX_XRES, Width);
    bga_write_register(VBE_DISPI_INDEX_YRES, Height);
    bga_write_register(VBE_DISPI_INDEX_BPP, BitDepth);
    bga_write_register(VBE_DISPI_INDEX_ENABLE,
                       VBE_DISPI_ENABLED | (use_lfb ? VBE_DISPI_LFB_ENABLED : 0)
                           | (clear ? 0 : VBE_DISPI_NOCLEARMEM));
}

void bga_set_bank(unsigned short BankNumber)
{
    cur_bank   = BankNumber;
    bank_start = cur_bank * bank_size;
    bank_end   = bank_start + bank_size;
    io_cli();
    bga_write_register(VBE_DISPI_INDEX_BANK, BankNumber);
    io_sti();
}