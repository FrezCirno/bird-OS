BOOT0_ADDR      equ 0x7c00

BOOT1_ADDR      equ 0x1000

KERNEL_PRELOAD_ADDR equ 0xf000

KERNEL_LOAD_ADDR    equ 0x100000

; 页目录开始地址:   2M
PDE_ADDR   equ 0x200000  

; 页表开始地址:    2M + 4K
PTE_ADDR   equ 0x201000  