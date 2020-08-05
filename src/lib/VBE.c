

u16 findMode(int x, int y, int d)
{
    struct VbeInfoBlock *ctrl = (VbeInfoBlock *)0x2000;
    struct ModeInfoBlock *inf = (ModeInfoBlock *)0x3000;
    u16 *modes;
    int i;
    u16 best = 0x13;
    int pixdiff, bestpixdiff     = DIFF(320 * 200, x * y);
    int depthdiff, bestdepthdiff = 8 >= d ? 8 - d : (d - 8) * 2;

    strncpy(ctrl->VbeSignature, "VBE2", 4);
    intV86(0x10, "ax,es:di", 0x4F00, 0, ctrl); // Get Controller Info
    if ((u16)v86.tss.eax != 0x004F) return best;

    modes = (u16 *)REALPTR(ctrl->VideoModePtr);
    for (i = 0; modes[i] != 0xFFFF; ++i)
    {
        intV86(0x10, "ax,cx,es:di", 0x4F01, modes[i], 0, inf); // Get Mode Info

        if ((u16)v86.tss.eax != 0x004F) continue;

        // Check if this is a graphics mode with linear frame buffer support
        if ((inf->attributes & 0x90) != 0x90) continue;

        // Check if this is a packed pixel or direct color mode
        if (inf->memory_model != 4 && inf->memory_model != 6) continue;

        // Check if this is exactly the mode we're looking for
        if (x == inf->XResolution && y == inf->YResolution
            && d == inf->BitsPerPixel)
            return modes[i];

        // Otherwise, compare to the closest match so far, remember if best
        pixdiff   = DIFF(inf->Xres * inf->Yres, x * y);
        depthdiff = (inf->bpp >= d) ? inf->bpp - d : (d - inf->bpp) * 2;
        if (bestpixdiff > pixdiff
            || (bestpixdiff == pixdiff && bestdepthdiff > depthdiff))
        {
            best          = modes[i];
            bestpixdiff   = pixdiff;
            bestdepthdiff = depthdiff;
        }
    }
    if (x == 640 && y == 480 && d == 1) return 0x11;
    return best;
}