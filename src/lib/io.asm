global io_hlt
global io_load_cr0, io_store_cr0
global io_load_eflags, io_store_eflags
global io_sti, io_cli
global in8, in16, in32
global out8, out16, out32

[BITS 32]
[SECTION .text]

io_hlt:
    hlt

io_sti:
    sti
    ret

io_cli:
    cli
    ret

io_load_eflags:
    pushfd
    pop eax
    ret

io_store_eflags:
    mov eax, [esp+4]
    push eax
    popfd
    ret

io_load_cr0:
    mov eax, cr0
    ret

io_store_cr0:
    mov eax, [esp+4]
    mov cr0, eax
    ret

in8:
    mov edx, [esp + 4]
    mov eax, 0
    in al, dx
    ret

in16:
    mov edx, [esp + 4]
    mov eax, 0
    in ax, dx
    ret

in32:
    mov edx, [esp + 4]
    in eax, dx
    ret

out8:
    mov edx, [esp + 4]
    mov al, [esp + 8]
    out dx, al
    ret

out16:
    mov edx, [esp + 4]
    mov ax, [esp + 8]
    out dx, ax
    ret

out32:
    mov edx, [esp + 4]
    mov eax, [esp + 8]
    out dx, eax
    ret