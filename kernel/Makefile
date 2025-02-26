CFLAGS := -mcmodel=large -I../include -I. -fno-builtin -fno-stack-protector -m64 -O0 -g \
		-fomit-frame-pointer -ffreestanding -nostdinc  -Wall

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


script/init.js.c: script/init.js
	xxd -i $^ $@

system: init.o js/engine.o ctype.o atoi.o script/init.js.o gdt.o stack.o
	$(LD) -T kernel.lds  -Bsymbolic -b elf64-x86-64 -nostdlib -static -o system $^
	$(NM) system > system.map

qemu: kernel.bin
	dd if=kernel.bin of=disk.img bs=512 seek=33 conv=notrunc
	qemu-system-x86_64 -smp 1 -m 2048 -boot c -hda ../build/disk.img -echr 0x07 -nographic

bochs: kernel.bin
	dd if=kernel.bin of=../build/disk.img bs=512 seek=33 conv=notrunc
	bochs -f ../bochs.txt

clean:
	@rm system system.map
	@find . -type f -name '*.o' | xargs rm
	@find . -type f -name '*.s' | xargs rm
	@find . -type f -name '*.img' | xargs rm
	@find . -type f -name '*.bin' | xargs rm
	@rm script/*.c
