#include <boot/bios.h>
#include <boot/link.h>
#include <serial/serial.h>
#include <size.h>

#define SERIAL_PORT SERIAL_DEFAULT_SERIAL_PORT

extern void pm_start();

struct disk_address_packet dap __attribute__((aligned(0x10))) = {
    .dap_size = 0x10,
    .count = 0,
    .address = 0,
    .segment = 0,
    .lba_low = 0,
    .lba_high = 0,
};

// 跨段复制
// from ds:si to es:di
static void __attribute__((noinline)) cp(unsigned long dst, unsigned long src,
                                         unsigned long count) {
  asm volatile("pushl %%ds\n\t"
               "pushl %%es\n\t"
               "mov %0, %%es\n\t"
               "mov %1, %%ds\n\t"
               "rep movsb\n\t"
               "popl  %%es\n\t"
               "popl  %%ds\n\t" ::"r"((dst >> 4) & 0xf000),
               "r"((src >> 4) & 0xf000), "c"(count),
               "S"((unsigned long)src & 0xffff),
               "D"((unsigned long)dst & 0xffff)
               :);
}

u64_t gdt[3] __attribute__((aligned(0x10))) = {
    [0] = 0,                  // reserved
    [1] = 0x00CF9A0000000000, // 32bit executable zero based no limit 4k page
    [2] = 0x00CF920000000000,
};

typedef struct {
  u16_t length;
  u32_t addr;
} __attribute__((packed)) gdt_ptr_t;

gdt_ptr_t gdt_ptr __attribute__((aligned(0x10))) = {
    .length = sizeof(gdt) - 1,
    .addr = 0,
};

#define SECS_PER_READ 4
static char cp_buf[SECS_PER_READ * SEC_SIZE];

// __entry 配合 linker script
// 强制把_start 里面的代码放到程序开头
void __attribute__((section("entry"))) _start() {
  gdt_ptr.addr = (u32_t)&gdt;
  asm volatile("cli");
  unsigned i, code = 0;

#define BUF_ADDR (unsigned long)(&cp_buf[0])
  // 先读到 buffer
  // 再跨段从 buffer copy 到 0x10000
  for (i = 0; i < KERNEL_SECS / SECS_PER_READ; i++) {
    dap.lba_low = i * SECS_PER_READ + 1 + LOADER_SECS;
    dap.address = BUF_ADDR;
    dap.count = SECS_PER_READ;
    code = bios_read_secs(&dap);

    cp(KERNEL_TMP + i * SECS_PER_READ * SEC_SIZE, BUF_ADDR,
       SECS_PER_READ * SEC_SIZE);

    if (code < 0) {
      serial_puts(SERIAL_PORT, "read sectors by bios call failed\n");
      while (1)
        ;
    }
  }

  serial_puts(SERIAL_PORT, "ready for detect memory map\n");

  unsigned long ebx = 0;
  void *dst = (void *)E820_MAP_ADDR;
  u16_t *e820_cnt = E820_MAP_LEN;
  *e820_cnt = 0;
  unsigned long cr0;

  // 查询 e820 map
  while (1) {
    code = e820_call(dst, &ebx);
    dst += 20;

    if (ebx == 0) {
      serial_puts(SERIAL_PORT, "e820 success\n");
      break;
    }

    if (code < 0) {
      serial_puts(SERIAL_PORT, "e820 error\n");
      while (1)
        ;
    }

    (*e820_cnt)++;
  }

  // 开启 a20
  if (inb(0x92) & 2) {
    serial_puts(SERIAL_PORT, "a20 is already enabled\n");
  } else {
    serial_puts(SERIAL_PORT, "a20 not enabled, go to enable it\n");
    outb_p(inb(0x92) | 2, 0x92);
  }

  // 加载 gdt
  serial_puts(SERIAL_PORT, "go on load gdt\n");
  asm volatile("lgdt %0" ::"m"(*&gdt_ptr) :);

  // 开启 cr0 保护位
  asm volatile("movl %%cr0, %0" : "=r"(cr0)::);
  cr0 |= 1;
  asm volatile("movl %0, %%cr0" ::"r"(cr0) :);

  serial_puts(SERIAL_PORT, "ready to enter protected mode\n");
  // ljmp 进入保护模式
  asm volatile("ljmp %0, %1" ::"i"(BOOT_INIT_CS), "i"(&pm_start));
}
