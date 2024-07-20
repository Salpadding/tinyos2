SHELL = bash

CFLAGS := -I../include -fno-builtin -fno-stack-protector \
		  -fomit-frame-pointer \
		  -ffreestanding -nostdinc \
		  -fno-asynchronous-unwind-tables \
		  -fno-pic -fno-dwarf2-cfi-asm -Wall

SECTOR_SIZE := $(shell awk '/#define SECTOR_SIZE/{print $$3}' ../include/boot/link.h)
LOADER_SECTORS := $(shell awk '/#define LOADER_SECTORS/{print $$3}' ../include/boot/link.h)
LOADER_FIRST_LBA := $(shell awk '/#define LOADER_FIRST_LBA/{print $$3}' ../include/boot/link.h)
PM_ENTRY := $(shell awk '/#define PM_ENTRY/{print $$3}' ../include/boot/link.h)

all: loader.bin mbr.bin

%.lds: %.ldS ../include/boot/link.h
	$(CPP) -P -include ../include/boot/link.h -o $@ $<

%.s: %.c
	$(CC) -S $(CFLAGS) -Os -m16  -o $@ $<


# 16bit mbr
mbr.o: mbr.c
	$(CC) -S $(CFLAGS) -Os -m16  -o $*.s $<
	$(AS) $(ASFLAGS) -o $@ $*.s

mbr.bin: mbr.o mbr.lds
	$(LD) -o mbr.elf -T mbr.lds mbr.o
	$(NM) mbr.elf > mbr.map
	$(OBJCOPY) -O binary mbr.elf $@
	dd if=mbr.bin of=../build/disk.img conv=notrunc bs=512 count=1

mbr.qemu: mbr.bin
	qemu-system-x86_64 -smp 1 -m 2048 -boot c -hda ../build/disk.img -echr 0x07 -nographic

# 16bit loader
loader.o: loader.c
	$(CC) -S $(CFLAGS) -Os -m16  -o $*.s $<
	$(AS) $(ASFLAGS) -o $@ $*.s


loader32.o: loader32.c
	$(CC) -S $(CFLAGS) -m32 -Os  -o $*.s $<
	$(AS) $(ASFLAGS) -o $@ $*.s

# 32bit pm code
pm.o: pm.c
	$(CC) -S $(CFLAGS) -m32 -Os  -o $*.s $<
	$(AS) $(ASFLAGS) -o $@ $*.s

8259a.o: 8259a.c
	$(CC) -S $(CFLAGS) -m32 -O0  -o $*.s $<
	$(AS) $(ASFLAGS) -o $@ $*.s


loader.bin: loader.o pm.o 8259a.o loader32.o loader.lds 
	$(LD) -o loader.elf -e _start -T loader.lds loader.o pm.o 8259a.o loader32.o
	$(NM) loader.elf > loader.map
	$(MAKE) -C ../tools/link
	touch $@
	../tools/link/link options.json
	dd if=loader.bin of=../build/disk.img seek=$(LOADER_FIRST_LBA) conv=notrunc bs=512 count=$(LOADER_SECTORS)

loader.qemu: loader.bin mbr.bin
	qemu-system-x86_64 -smp 1 -m 2048 -boot c -hda ../build/disk.img -echr 0x07 -nographic

bochs: loader.bin mbr.bin
	bochs -f ../bochs.txt

clean:
	@rm *.bin *.o *.s *.elf *.map *.lds

