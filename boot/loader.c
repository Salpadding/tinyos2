#define ATTRIBUTES                                                             \
  __attribute((noinline)) __attribute__((section("header.text")))
#include <boot/bios.h>
#include <boot/link.h>

#include <boot/loader.h>

#include <serial/serial.h>

#include <size.h>

#define printf(x)

__attribute__((section("header.data")))
__attribute__((aligned(0x10))) uint64_t gdt[4] = {
    [0] = 0, // reserved
    [1] =
        0x00CF9A0000000000, // 32bit executable zero based no limit 4k page // 8
    [2] = 0x00CF920000000000, // 16
};

typedef struct {
  uint16_t length;
  uint32_t addr;
} __attribute__((packed)) gdt_ptr_t;

__attribute__((section("header.data")))
__attribute__((aligned(0x10))) gdt_ptr_t gdt_ptr = {
    .length = sizeof(gdt) - 1,
    .addr = 0,
};

__attribute__((section("header.data")))
__attribute__((aligned(0x10))) char e820_table[PAGE_SIZE];

__attribute__((section("header.data")))
__attribute__((aligned(0x10))) uint16_t e820_count = 0;

__attribute__((section("header.entry"))) void _start() {
  asm volatile("mov %%ax, %%sp\n\t"
               "mov %%ax, %%bp\n\t" ::"a"(BOOT_SP)
               :);

  gdt_ptr.addr = (uint32_t)&gdt;
  asm volatile("cli");
  unsigned i, code = 0;

  printf("ready for detect memory map\n");

  unsigned long ebx = 0;
  void *dst = (void *)(e820_table);
  e820_count = 0;
  unsigned long cr0;

  // 查询 e820 map
  while (1) {
    code = e820_call(dst, &ebx);
    dst += 20;

    if (ebx == 0) {
      printf("e820 success\n");
      break;
    }

    if (code < 0) {
      printf("e820 error\n");
      while (1)
        ;
    }
    e820_count++;
  }

  // 开启 a20
  if (inb(0x92) & 2) {
    printf("a20 is already enabled\n");
  } else {
    printf("a20 not enabled, go to enable it\n");
    outb_p(inb(0x92) | 2, 0x92);
  }

  // 加载 gdt
  printf("go on load gdt\n");
  asm volatile("lgdt %0" ::"m"(*(&gdt_ptr)) :);

  // 开启 cr0 保护位
  asm volatile("movl %%cr0, %0" : "=r"(cr0)::);
  cr0 |= 1;
  asm volatile("movl %0, %%cr0" ::"r"(cr0) :);

  printf("jump to loader32\n");

  asm volatile("jump_to_loader32:\n\tjmp %0, %1" ::"i"(BOOT_INIT_CS),
               "i"(&loader32));
}
