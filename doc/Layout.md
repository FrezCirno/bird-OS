## 软盘数据布局

| Cylinder | Head | Sector | 数据     |
|----------|------|--------|----------|
| 0        | 0    | 1      | boot0    |
| 0        | 0    | 2..18  | boot1    |
| 0        | 1    | 1..18  | (unused) |
| 1..32    | 0..1 | 1..18  | kernel(max: 32\*2\*18\*512 = 576K)   |
| 33..79   | 0..1 | 1..18  | (unused) |

## 内存数据布局

| Linear Address | Item                                |
|----------------|-------------------------------------|
| FFFFFFFF(4G)   | max top memory                      |
| 00xxxxxx       | kernel's BSS section                |
| 00xxxxxx       | kernel's DATA section               |
| 00100000(1M)   | kernel's TEXT section               |
| 000F0000       | Video memory, MMIO, BIOS            |
| 000A0000       | Bottom of memory hole               |
| 00090000       | kernel preload (end: 0x9f000)       |
| 00010000       | kernel preload (size: 0x90000)      |
| 0000F000       | kernel preload (start: 0xf000)      |
| 0000E000       | Protected mode stack (top: 0xf000)  |
| 0000D000       | :                                   |
| 0000C000       | Disk buffer (end: 0xc7ff)           |
| 0000B000       | :                                   |
| 0000A000       | :                                   |
| 00009000       | :                                   |
| 00008000       | Disk buffer (start: 0x8000)         |
| 00007000       | boot0 (0x7c00–0x7dff)               |
| 00006000       | Real mode stack (top: 0x7000)       |
| 00005000       | :                                   |
| 00004000       | :                                   |
| 00003000       | boot1 (end: 0x31ff)                 |
| 00002000       | :                                   |
| 00001000(4K)   | boot1 (start: 0x1000)               |
| 00000000       | Reserved (real mode IVT, BIOS data) |