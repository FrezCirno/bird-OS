#include <types.h>
#include <memory.h> // memtest_sub
#include <io.h>     // io_*

u32 memtest(u32 start, u32 end)
{
    u8 flg486 = 0;
    u32 eflg, cr0;

    /* 确认CPU是386还是486以上的 */
    eflg = io_load_eflags();
    eflg |= EFLAGS_AC_BIT; /* AC-bit = 1 */
    io_store_eflags(eflg);
    eflg = io_load_eflags();
    if ((eflg & EFLAGS_AC_BIT) != 0)
    {
        /* 如果是386，即使设定AC=1，AC的值还会自动回到0 */
        flg486 = 1;
    }

    eflg &= ~EFLAGS_AC_BIT; /* AC-bit = 0 */
    io_store_eflags(eflg);

    if (flg486 != 0)
    {
        cr0 = io_load_cr0();
        cr0 |= CR0_CACHE_DISABLE; /* 禁止缓存 */
        io_store_cr0(cr0);
    }

    u32 i = memtest_sub(start, end);

    if (flg486 != 0)
    {
        cr0 = io_load_cr0();
        cr0 &= ~CR0_CACHE_DISABLE; /* 允许缓存 */
        io_store_cr0(cr0);
    }

    return i;
}
