CFLAGS := -mcmodel=large -I../include -I. -fno-builtin -fno-stack-protector -m64 -Og -g -fomit-frame-pointer -ffreestanding -nostdinc  -Wall

ASFLAGS := --64


%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

%.s: %.c
	$(CC) -S $(CFLAGS) -o $@ $<

%.o: %.S
	$(CPP) -D__ASM__ -Iinclude -o $*.s $<
	$(AS) $(ASFLAGS) -o $@ $*.s


kernel.bin: system
	$(OBJCOPY) -I elf64-x86-64 -S -R ".eh_frame" -R ".comment" -O binary system kernel.bin

system: init.o js/engine.o
	$(LD)  -T kernel.lds  -Bsymbolic -b elf64-x86-64 -nostdlib -static -o system $^

qemu: kernel.bin
	dd if=kernel.bin of=disk.img bs=512 seek=33 conv=notrunc
	qemu-system-x86_64 -smp 1 -m 2048 -boot c -hda ../build/disk.img -echr 0x07 -nographic

bochs: kernel.bin
	dd if=kernel.bin of=../build/disk.img bs=512 seek=33 conv=notrunc
	bochs -f ../bochs.txt
