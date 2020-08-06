BOOT0_ADDR      equ 0x7c00

BOOT1_ADDR      equ 0x1000

KERNEL_PRELOAD_ADDR equ 0xf000

KERNEL_LOAD_ADDR    equ 0x100000

; 页目录开始地址:   0
; 结束地址 0x1000
; 每个PDE指向1k个PTE
PDE_ADDR   equ   0x200000

; 页表开始地址:   0x201000 4K
; 最大 0x400000
; 每个PTE映射1个4k页面
PTE_ADDR   equ   0x201000  