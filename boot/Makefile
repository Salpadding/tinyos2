CFLAGS := -I../include -fno-builtin -fno-stack-protector \
		  -Os -fomit-frame-pointer \
		  -ffreestanding -nostdinc \
		  -fno-asynchronous-unwind-tables \
		  -fno-pic -fno-dwarf2-cfi-asm -Wall 

all: loader.bin mbr.bin
	dd if=mbr.bin of=../build/disk.img conv=notrunc
	dd if=loader.bin of=../build/disk.img seek=1 conv=notrunc


# 16bit mbr
mbr.o: mbr.c
	$(CC) -S $(CFLAGS) -m16  -o $*.s $<
	$(AS) $(ASFLAGS) -o $@ $*.s

mbr.bin: mbr.o mbr.c
	$(LD) -o mbr.elf -T mbr.lds mbr.o
	nm mbr.elf > mbr.map
	$(OBJCOPY) -O binary mbr.elf $@

mbr.qemu: mbr.bin mbr.c
	dd if=mbr.bin of=../build/disk.img conv=notrunc
	qemu-system-x86_64 -smp 1 -m 2048 -boot c -hda ../build/disk.img -echr 0x07 -nographic

# 16bit loader
loader.o: loader.c
	$(CC) -S $(CFLAGS) -m16  -o $*.s $<
	$(AS) $(ASFLAGS) -o $@ $*.s


# 32bit pm code
pm.o: pm.c
	$(CC) -S $(CFLAGS) -m32  -o $*.s $<
	$(AS) $(ASFLAGS) -o $@ $*.s


loader.bin: loader.o pm.o loader.c pm.c
	$(LD) -o loader.elf -e _start -T loader.lds loader.o pm.o
	$(NM) loader.elf > loader.map
	$(OBJCOPY) -O binary loader.elf $@

loader.qemu: loader.bin mbr.bin
	dd if=mbr.bin of=../build/disk.img conv=notrunc
	dd if=loader.bin of=../build/disk.img seek=1 conv=notrunc
	qemu-system-x86_64 -smp 1 -m 2048 -boot c -hda ../build/disk.img -echr 0x07 -nographic

bochs: loader.bin mbr.bin
	dd if=mbr.bin of=../build/disk.img conv=notrunc
	dd if=loader.bin of=../build/disk.img seek=1 conv=notrunc
	bochs -f ../bochs.txt

clean:
	@rm *.bin *.o *.s *.elf *.map
