;
; boot1.asm
;

%include "pm.inc"
%include "load.inc"

    org BOOT1_ADDR

[SECTION .textR]
    ; 初始化寄存器和栈
    mov ax, 0
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7c00

    ; 查询BIOS内存容量
MemChk:
    xor ebx, ebx   ; ebx = 后续值, 开始时需为 0
    mov di, MemChkBuf  ; es:di 指向目标地址
MemChkLoop:
    mov eax, 0xE820      ; 查询系统地址映射
    mov ecx, 20          ; 请求20个字节
    ; 地址范围描述符结构 (Address Range Descriptor Structure), 大小为20个字节
    mov edx, 0x534D4150  ; 'SMAP'
    int 15h
    jc MemChkFail  ; carry set on first call means "unsupported function", then means "end of list already reached"
    add di, 20
    inc dword [dwMCRNumber] ; dwMCRNumber = ARDS 的个数
    test ebx, ebx  ;  列表到头了
    jne MemChkLoop
    jmp MemChkOK
MemChkFail:
    mov dword [dwMCRNumber], 0
MemChkOK:


    ; 将位于1-0-1到32-1-18共576K/0x90000K的数据(kernel)加载到内存0xf000-0x9f000处
    mov ax, 0
    mov es, ax ; 目标地址
    mov bx, KERNEL_PRELOAD_ADDR ; 目标地址
    mov dl, 0 ; 驱动器号
    mov ch, 1 ; 磁道号(0-79)
    mov dh, 0 ; 柱面号(0-1)
    mov cl, 1 ; 扇区号(1-18)
    mov al, 1 ; 扇区数量(<=19-cl)

next_sector:
    call ReadSector
    inc cl
    add bx, 0x0200 ; 512
    cmp bx, 0
    jne not_overflow
    ; bx = 0xffff -> 0 溢出
    mov bx, es
    add bx, 0x1000
    mov es, bx
    mov bx, 0
not_overflow:
    cmp cl, 18
    jg next_head
    jmp next_sector
next_head:
    mov cl, 1 ; 扇区
    inc dh
    cmp dh, 1
    jg next_cyl
    jmp next_sector
next_cyl:
    mov dh, 0
    inc ch
    cmp ch, 32
    jg fin
    jmp next_sector
fin:

    ; 设置video mode为图形模式
    ; mov ax, 0x0013  ;  VGA, 320x200x256彩色
    mov ax, 0x4f02  ; VBE模式
    mov bx, 0x0103  ; 800x600x256彩色
    int 0x10

    ; 打开A20地址线
    ; 这里通过BIOS
    mov ax, 0x2401 ; enable A20
    int 0x15

    ; 关中断
    cli

    ; 加载GDT
    lgdt [gdtptr]

    ; 进入保护模式/32位模式
    mov eax, cr0
    or eax, 1
    mov cr0, eax

    ; 通过远跳转更新cs和eip
    jmp dword 8:protect


%include "lib16.inc"

error:
    cli ; 再次关中断
    mov ax, ds
    mov es, ax     ; es = ds
    mov ax, msg_error
    mov bp, ax     ; bp = msg
    mov cx, msg_error_end - msg_error
    call PrintLine
error1:
    hlt
    jmp error1

; -------------------------------------------------------
; 以下进入保护模式, 所有绝对数据地址都需要手动加上模块基地址
; -------------------------------------------------------
[BITS 32]
[SECTION .textP]
protect:
    mov ax, 0x18
    mov gs, ax
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov ss, ax
    mov esp, 0xf000

    call GetMemSize
    call SetupPaging
    call LoadKernel

    ; jump to os kernel
    jmp 8:KERNEL_LOAD_ADDR

; ---------------------------------------------------------------------------------
; ELF格式可执行文件加载
; 遍历每一个 Program Header，根据 Program Header 中的信息来确定把什么放进内存，放到什么位置，以及放多少。
; --------------------------------------------------------------------------------------------
LoadKernel:
    xor esi, esi
    ; ecx = Elf32_Ehdr->e_phnum
    mov cx, word [KERNEL_PRELOAD_ADDR + 0x2C]
    movzx ecx, cx  ; 零拓展
    ; esi = KERNEL_PRELOAD_ADDR + Elf32_Ehdr->e_phoff = Elf32_Phdr
    mov esi, [KERNEL_PRELOAD_ADDR + 0x1C]
    add esi, KERNEL_PRELOAD_ADDR
.Begin:
    mov eax, [esi + 0] ; Elf32_Phdr->p_type
    cmp eax, 0    ; PT_NULL
    jz .NoAction
    ; MemCpy(dst=Elf32_Phdr->p_vaddr, src=KERNEL_PRELOAD_ADDR + Elf32_Phdr->p_offset, Elf32_Phdr->p_filesz)
    push dword [esi + 0x10] ; Elf32_Phdr->p_filesz
    mov eax, [esi + 0x4]
    add eax, KERNEL_PRELOAD_ADDR
    push eax ; Elf32_Phdr->p_offset + KERNEL_PRELOAD_ADDR
    push dword [esi + 0x8]  ; Elf32_Phdr->p_vaddr 链接的时候设置的-Ttext即是设置这个值
    call MemCpy
    add esp, 12
.NoAction:
    add esi, 0x20   ; esi += Elf32_Ehdr->e_phentsize, 20字节
    dec ecx
    jnz .Begin
    ret
; --------------------------------------------------------------------------------------------
; 显示内存信息
; --------------------------------------------------------------------------------------------
GetMemSize:
    push esi
    push edi
    push ecx

    mov esi, MemChkBuf
    mov ecx, [dwMCRNumber] ;for(int i=0;i<[MCRNumber];i++) // 每次得到一个ARDS(Address Range Descriptor Structure)结构
.3: ;{
    mov edx, 5 ; for(int j=0;j<5;j++) // 每次得到一个ARDS中的成员，共5个成员
    mov edi, ARDStruct ; { // 依次显示：BaseAddrLow，BaseAddrHigh，LengthLow，LengthHigh，Type
.1: ;
    push dword [esi] ;
    
    pop eax ;
    stosd ; ARDStruct[j*4] = MemChkBuf[j*4];
    add esi, 4 ;
    dec edx ;
    cmp edx, 0 ;
    jnz .1 ; }
    
    cmp dword [dwType], 1 ; if(Type == AddressRangeMemory) // AddressRangeMemory : 1, AddressRangeReserved : 2
    jne .2 ; {
    mov eax, [dwBaseAddrLow] ;
    add eax, [dwLengthLow] ;
    cmp eax, [dwMemSize] ; if(BaseAddrLow + LengthLow > MemSize)
    jb .2 ;
    mov [dwMemSize], eax ; MemSize = BaseAddrLow + LengthLow;
.2: ; }
    loop .3 ;}
    
    pop ecx
    pop edi
    pop esi
    ret

; ---------------------------------------------------------------------------
; 启动分页机制
; ------------------------------------------------------------------------
SetupPaging:
    ; 根据内存大小计算应初始化多少PDE以及多少页表
    xor edx, edx
    mov eax, [dwMemSize]
    mov ebx, 400000h ; 400000h = 4M = 4096 * 1024, 一个页表对应的内存大小
    div ebx
    mov ecx, eax ; 此时 ecx 为页表的个数，也即 PDE 应该的个数
    test edx, edx
    jz .no_remainder
    inc ecx ; 如果余数不为 0 就需增加一个页表
.no_remainder:
    push ecx ; 暂存页表个数

    ; 为简化处理, 所有线性地址对应相等的物理地址. 并且不考虑内存空洞.

    ; 首先初始化页目录
    mov ax, 0x10
    mov es, ax
    mov edi, PDE_ADDR ; 此段首地址为 PDE_ADDR
    xor eax, eax
    mov eax, PTE_ADDR | PG_P | PG_RW
.1:
    stosd  ;  es[edi++] = eax
    add eax, 0x1000 ; 为了简化, 所有页表在内存中是连续的.
    loop .1

    ; 再初始化所有页表
    pop eax ; 页表个数
    mov ebx, 1024 ; 每个页表 1024 个 PTE
    mul ebx
    mov ecx, eax ; PTE个数 = 页表个数 * 1024
    mov edi, PTE_ADDR ; 此段首地址为 PTE_ADDR
    xor eax, eax
    mov eax, PG_P | PG_RW
.2:
    stosd
    add eax, 4096 ; 每一页指向 4K 的空间
    loop .2

    mov eax, PDE_ADDR
    mov cr3, eax
    mov eax, cr0
    or eax, 80000000h
    mov cr0, eax
    jmp short .3
.3:
    nop
    ret
; ------------------------------------------------------------------------
; void* MemCpy(void* dst, void* src, int size);
; ------------------------------------------------------------------------
MemCpy:
    push ebp
    mov ebp, esp

    push esi
    push edi
    push ecx

    mov edi, [ebp + 8] ; dst(8)
    mov esi, [ebp + 12] ; src(4)
    mov ecx, [ebp + 16] ; size(4)
.1:
    cmp ecx, 0
    jz .2

    mov al, [ds:esi]
    inc esi

    mov byte [es:edi], al
    inc edi

    dec ecx
    jmp .1
.2:
    mov eax, [ebp + 8]

    pop ecx
    pop edi
    pop esi
    mov esp, ebp
    pop ebp

    ret  

; --------------------------------------------------------------------------------------------
[SECTION .data]
ALIGN 32

dwMCRNumber:   dd   0 ; Memory Check Result
dwMemSize:     dd   0
ARDStruct:   ; Address Range Descriptor Structure
    dwBaseAddrLow:    dd   0
    dwBaseAddrHigh:   dd   0
    dwLengthLow:      dd   0
    dwLengthHigh:     dd   0
    dwType:           dd   0
MemChkBuf:  times 256 db   0

gdt:   Descriptor  0, 0, 0 ; 8B 0x111d
       Descriptor  0, 0fffffh, DA_G | DA_D | DA_P | DA_DPL0 | DA_CXR   ; 4GB 32-bit DPL=0 内核代码段
       Descriptor  0, 0fffffh, DA_G | DA_D | DA_P | DA_DPL0 | DA_DRW   ; 4GB 32-bit DPL=0 内核数据段
       Descriptor  0xa0000, 0xffff, DA_D | DA_P | DA_DPL3 | DA_DRW  ;  显存数据段

; gdtr 结构
gdtptr   dw   $ - gdt - 1 ; 2字节: sizeof(gdt) - 1
         dd   gdt  ; 4字节: gdt基地址
msg_error  db  "Error!"
msg_error_end:
