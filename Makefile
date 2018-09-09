#########################
# Makefile for Orange'S #
#########################

# Entry point of Orange'S
# It must have the same value with 'KernelEntryPointPhyAddr' in load.inc!
ENTRYPOINT	= 0x1000

# Offset of entry point in kernel file
# It depends on ENTRYPOINT
ENTRYOFFSET	=   0x400

# Programs, flags, etc.
ASM		= nasm
DASM		= objdump
CC		= gcc
LD		= ld
ASMBFLAGS	= -I boot/include/
ASMKFLAGS	= -I include/ -I include/sys/ -f elf
CFLAGS		= -I include/ -I include/sys/ -c -fno-builtin -Wall -fno-stack-protector
#CFLAGS		= -I include/ -c -fno-builtin -fno-stack-protector -fpack-struct -Wall
LDFLAGS		= -Ttext $(ENTRYPOINT) -Map krnl.map
DASMFLAGS	= -D
COM		= -m elf_i386

# This Program
ORANGESBOOT	= boot/boot.bin boot/loader.bin
ORANGESKERNEL	= kernel.bin
OBJS		= kernel/kernel.o lib/syscall.o kernel/start.o kernel/main.o\
			kernel/clock.o kernel/keyboard.o kernel/tty.o kernel/console.o\
			kernel/i8259.o kernel/global.o kernel/protect.o kernel/proc.o\
			kernel/systask.o kernel/hd.o\
			lib/printf.o lib/vsprintf.o\
			lib/kliba.o lib/klib.o lib/string.o lib/misc.o\
			lib/open.o lib/read.o lib/write.o lib/close.o lib/unlink.o\
			lib/getpid.o lib/syslog.o\
			fs/main.o fs/open.o fs/misc.o fs/read_write.o\
			fs/link.o\
			fs/disklog.o
DASMOUTPUT	= kernel.bin.asm

# All Phony Targets
.PHONY : everything final image clean realclean disasm all buildimg

# Default starting position
nop :
	@echo "why not \`make image' huh? :)"

everything : $(ORANGESBOOT) $(ORANGESKERNEL)

all : realclean everything

image : realclean everything clean buildimg

clean :
	rm -f $(OBJS)

realclean :
	rm -f $(OBJS) $(ORANGESBOOT) $(ORANGESKERNEL)

disasm :
	$(DASM) $(DASMFLAGS) $(ORANGESKERNEL) > $(DASMOUTPUT)

# We assume that "a.img" exists in current folder
buildimg :
	dd if=boot/boot.bin of=a.img bs=512 count=1 conv=notrunc
	sudo mount -o loop a.img /mnt/floppy/
	sudo cp -fv boot/loader.bin /mnt/floppy/
	sudo cp -fv kernel.bin /mnt/floppy
	sudo umount /mnt/floppy

boot/boot.bin : boot/boot.asm boot/include/load.inc boot/include/fat12hdr.inc
	$(ASM) $(ASMBFLAGS) -o $@ $<

boot/loader.bin : boot/loader.asm boot/include/load.inc boot/include/fat12hdr.inc boot/include/pm.inc
	$(ASM) $(ASMBFLAGS) -o $@ $<

$(ORANGESKERNEL) : $(OBJS)
	$(LD) $(COM) $(LDFLAGS) -o $(ORANGESKERNEL) $(OBJS)

kernel/kernel.o : kernel/kernel.asm
	$(ASM) $(ASMKFLAGS) -o $@ $<

lib/syscall.o : lib/syscall.asm
	$(ASM) $(ASMKFLAGS) -o $@ $<

kernel/start.o: kernel/start.c
	$(CC) -m32 $(CFLAGS) -o $@ $<

kernel/main.o: kernel/main.c
	$(CC) -m32 $(CFLAGS) -o $@ $<

kernel/clock.o: kernel/clock.c
	$(CC) -m32 $(CFLAGS) -o $@ $<

kernel/keyboard.o: kernel/keyboard.c
	$(CC) -m32 $(CFLAGS) -o $@ $<

kernel/tty.o: kernel/tty.c
	$(CC) -m32 $(CFLAGS) -o $@ $<

kernel/console.o: kernel/console.c
	$(CC) -m32 $(CFLAGS) -o $@ $<

kernel/i8259.o: kernel/i8259.c
	$(CC) -m32 $(CFLAGS) -o $@ $<

kernel/global.o: kernel/global.c
	$(CC) -m32 $(CFLAGS) -o $@ $<

kernel/protect.o: kernel/protect.c
	$(CC) -m32 $(CFLAGS) -o $@ $<

kernel/proc.o: kernel/proc.c
	$(CC) -m32 $(CFLAGS) -o $@ $<

lib/printf.o: lib/printf.c
	$(CC) -m32 $(CFLAGS) -o $@ $<

lib/vsprintf.o: lib/vsprintf.c
	$(CC) -m32 $(CFLAGS) -o $@ $<

kernel/systask.o: kernel/systask.c
	$(CC) -m32 $(CFLAGS) -o $@ $<

kernel/hd.o: kernel/hd.c
	$(CC) -m32 $(CFLAGS) -o $@ $<

lib/klib.o: lib/klib.c
	$(CC) -m32 $(CFLAGS) -o $@ $<

lib/misc.o: lib/misc.c
	$(CC) -m32 $(CFLAGS) -o $@ $<

lib/kliba.o : lib/kliba.asm
	$(ASM) $(ASMKFLAGS) -o $@ $<

lib/string.o : lib/string.asm
	$(ASM) $(ASMKFLAGS) -o $@ $<

lib/open.o: lib/open.c
	$(CC) -m32 $(CFLAGS) -o $@ $<

lib/read.o: lib/read.c
	$(CC) -m32 $(CFLAGS) -o $@ $<

lib/write.o: lib/write.c
	$(CC) -m32 $(CFLAGS) -o $@ $<

lib/close.o: lib/close.c
	$(CC) -m32 $(CFLAGS) -o $@ $<

lib/unlink.o: lib/unlink.c
	$(CC) -m32 $(CFLAGS) -o $@ $<

lib/getpid.o: lib/getpid.c
	$(CC) -m32 $(CFLAGS) -o $@ $<

lib/syslog.o: lib/syslog.c
	$(CC) -m32 $(CFLAGS) -o $@ $<

fs/main.o: fs/main.c
	$(CC) -m32 $(CFLAGS) -o $@ $<

fs/misc.o: fs/misc.c
	$(CC) -m32 $(CFLAGS) -o $@ $<

fs/open.o: fs/open.c
	$(CC) -m32 $(CFLAGS) -o $@ $<

fs/read_write.o: fs/read_write.c
	$(CC) -m32 $(CFLAGS) -o $@ $<

fs/link.o: fs/link.c
	$(CC) -m32 $(CFLAGS) -o $@ $<

fs/disklog.o: fs/disklog.c
	$(CC) -m32 $(CFLAGS) -o $@ $<

