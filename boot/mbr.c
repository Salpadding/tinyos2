#define ATTRIBUTES __attribute__((noinline))

#include <boot/bios.h>
#include <boot/debug.h>
#include <boot/link.h>
#include <x86/8086.h>

#define SECS_PER_READ 4

#ifdef DEBUG_MBR
#include <serial/serial.h>
#define printf(x) serial_puts(SERIAL_DEFAULT_SERIAL_PORT, x)
#else
#define printf(x)
#endif

static void boot();

__attribute__((section("entry"))) void _start() {
  // 关闭所有外中断
  asm volatile("cli\n\t");

  // reset all
  asm volatile("mov %%ax, %%ds\n\t"
               "mov %%ax, %%es\n\t"
               "mov %%ax, %%fs\n\t"
               "mov %%ax, %%gs\n\t"
               "mov %%ax, %%ss\n\t" ::"a"(0));

  //  复位栈寄存器
  asm volatile("mov %%ax, %%sp\n\t"
               "mov %%ax, %%bp\n\t" ::"a"(BOOT_SP)
               :);

  boot();
}

__attribute__((noinline)) static void
cp(struct disk_address_packet *dap, unsigned long dst, unsigned long total) {
  unsigned long code;
  while (total >= dap->sectors) {
    printf("read 4 sector from disk\n");
    dap->offset = _8086_OFFSET(dst);
    dap->segment = _8086_SEGMENT(dst);
    code = bios_read_secs(dap);

    if (code < 0) {
      printf("read sectors by bios call failed\n");
      while (1)
        ;
    }

    dap->lba_low += dap->sectors;
    dst += dap->sectors * SECTOR_SIZE;
    total -= dap->sectors;
  }
}

__attribute__((noinline)) void jmp_loader() {
  printf("\n Starting 8086... jump to header of loader\n");
  asm volatile("jmp %0, %1" ::"i"(_8086_SEGMENT(HEADER_ENTRY)),
               "i"(_8086_OFFSET(HEADER_ENTRY))
               :);
}

static void boot() {
  unsigned long i;

  if (sizeof(struct disk_address_packet) != 0x10) {
    printf("dap assert failed\n");
    while (1) {
    }
  }

  struct disk_address_packet dap __attribute__((aligned(0x20))) = {
      .dap_size = 0x10, // sizeof this structure
      .reserved = 0,
      .sectors = SECS_PER_READ,
      .offset = 0,
      .segment = 0,
      .lba_low = 0,
      .lba_high = 0,
  };

  // 初始化串口
#ifdef DEBUG_MBR
  INIT_SERIAL(SERIAL_DEFAULT_SERIAL_PORT);
#endif
  dap.lba_low = LOADER_FIRST_LBA;
  cp(&dap, HEADER_ENTRY, HEADER_MAX_SECTORS);
  dap.lba_low = LOADER_FIRST_LBA;
  cp(&dap, PM_ENTRY, LOADER_SECTORS);

  printf("\nread boot loader from disk\n");
  jmp_loader();
}
