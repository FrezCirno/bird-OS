
# Programs, flags, etc.
ASM  = nasm
DASM = ndisasm
CC   = gcc
LD   = ld

CFLAGS = -m32 -fno-builtin -nostdlib -nostdinc -Iinclude -c -g

default: all

# rules

.c.o:
	$(CC) $(CFLAGS) -o $@ $<

.asm.o:
	$(ASM) -f elf -o $@ $<


# boot

boot/boot0.bin : boot/boot0.asm boot/include/
	$(ASM) -I boot/include/ -o $@ $<

boot/boot1.bin : boot/boot1.asm boot/include/
	$(ASM) -I boot/include/ -o $@ $<


# kernel

init/_start.o: init/_start.asm
	$(ASM) -f elf -o $@ $<

int/int_entry.o: int/int_entry.asm
	$(ASM) -f elf -I include/ -o $@ $<

int/int_ctl.o: int/int_ctl.asm
	$(ASM) -f elf -I include/ -o $@ $<

kernel/restart.o: kernel/restart.asm
	$(ASM) -f elf -I include/ -o $@ $<

lib/io.o: lib/io.asm
	$(ASM) -f elf -o $@ $<

lib/syscall.o: lib/syscall.asm
	$(ASM) -f elf -o $@ $<

mm/memtest_sub.o: mm/memtest_sub.asm
	$(ASM) -f elf -o $@ $<

kernel.o : init/_start.o init/main.o \
		int/int_entry.o int/irqh.o \
		kernel/protect.o kernel/proc.o kernel/sched.o \
		kernel/traps.o kernel/syscall.o \
		kernel/clock.o kernel/keyboard.o kernel/mouse.o \
		kernel/drv/hd.o kernel/drv/ata.o \
		kernel/errno.o kernel/panic.o kernel/assert.o \
		lib/glib.o lib/ctype.o \
		lib/itoa.o lib/u_vga16.o lib/BGA.o lib/buffer.o \
		mm/memory.o 
	$(LD) -m elf_i386 -Ttext 0x100000 -o $@ $^


# image

build : build.img

build.img : boot/boot0.bin boot/boot1.bin kernel.o
	dd if=/dev/zero of=build.img count=2880
	dd if=boot/boot0.bin of=build.img count=1 conv=notrunc
	dd if=boot/boot1.bin of=build.img count=17 seek=1 conv=notrunc
	dd if=kernel.o of=build.img count=1152 seek=36 conv=notrunc


all : build dump qemu

bochs :
	echo c | bochs

dump: build
	objdump -M intel kernel.o -d > dump.txt

clean :
	-rm -f *.o boot/*.bin init/*.o int/*.o kernel/*.o lib/*.o mm/*.o
	-rm -f build.img

qemu:
	qemu-system-i386                                 \
	  -accel tcg,thread=single                       \
	  -cpu core2duo                                  \
	  -m 512                                         \
	  -no-reboot                                     \
	  -fda build.img                                 \
      -drive format=raw,file=80m.img                 \
	  -gdb tcp::1234                                 \
	  -S                                             \
	  -smp 1                                         \
	  -vga std &
	#   -serial stdio                                  \
	#   -usb                                          
	gdb

rebuild: clean build