; 用户程序
; 系统调用库
; syscall.h

INT_VECTOR_SYS_CALL   equ   0x90

global	syscall ; 导出符号

[section .text]
[bits 32]

syscall: ; int syscall(u32 id); ; 0x101a90
	mov	eax, [esp + 4] ; id
	int	INT_VECTOR_SYS_CALL
	ret