#include <boot/bios.h>
#include <boot/link.h>
#include <serial/serial.h>

#define SERIAL_PORT SERIAL_DEFAULT_SERIAL_PORT

struct disk_address_packet dap __attribute__((aligned(0x10))) = {
    .dap_size = 0x10,
    .count = 0,
    .address = 0,
    .segment = 0,
    .lba_low = 0,
    .lba_high = 0,
};

// __entry 配合 boot.lds 强制让 _start 位于 .text 的开始
void __attribute__((section("entry"))) _start() {
  // reset all
  asm volatile("mov %%ax, %%ds\n\t"
               "mov %%ax, %%es\n\t"
               "mov %%ax, %%fs\n\t"
               "mov %%ax, %%gs\n\t"
               "mov %%ax, %%ss\n\t" ::"a"(0));

  unsigned long i;

  //  复位栈寄存器
  asm volatile("mov %%ax, %%sp\n\t"
               "mov %%ax, %%bp\n\t" ::"a"(0xfff0)
               :);

  // 初始化串口
  INIT_SERIAL(SERIAL_PORT);

  unsigned long code;

#define SECS_PER_READ 4
  // 每次读取 512 byte, 重复32次

  serial_puts(SERIAL_PORT, "\nread boot loader from disk\n");
  for (i = 0; i < LOADER_SECS / SECS_PER_READ; i++) {
    dap.lba_low = i * SECS_PER_READ + 1;
    dap.address = i * SECS_PER_READ * SEC_SIZE + LOADER_ENTRY;
    dap.count = SECS_PER_READ;
    code = bios_read_secs(&dap);

    if (code < 0) {
      serial_puts(SERIAL_PORT, "read sectors by bios call failed\n");
      while (1)
        ;
    }
  }

jmp_loader:
  serial_puts(SERIAL_PORT, "\n Starting 8086... jump to loader\n");
  asm volatile("jmp $0, %0" ::"i"(LOADER_ENTRY) :);
}
