#ifndef _BOOT_BIOS_H
#define _BOOT_BIOS_H

#ifdef SECTION_NAME
#define ATTRIBUTES __attribute__((section(SECTION_NAME)))
#endif

#ifndef ATTRIBUTES
#define ATTRIBUTES
#endif

struct __attribute__((packed)) disk_address_packet {
  unsigned char dap_size; // always 16
  unsigned char reserved; // always 0
  unsigned short sectors;
  unsigned short offset;
  unsigned short segment;
  unsigned long lba_low;
  unsigned long lba_high;
};

// ah=0x42 lba read
// 0x80: first drive
ATTRIBUTES
static int bios_read_secs(struct disk_address_packet *dap) {
  unsigned long eflags;
  asm volatile("int $0x13\n\t"
               "pushfl\n\t"
               "popl %0\n\t"
               : "=r"(eflags)
               : "d"(0x80), "S"((unsigned long)dap), "a"(0x4200)
               );

  if (eflags & 1) {
    return -1;
  }
  return 0;
}

// 这个函数通常会在循环里被调用
ATTRIBUTES
static int e820_call(void *dst, unsigned long *ebx) {
#define EFLAGS_CF 1
  unsigned long eflags;
  asm volatile("movl %1, %%ebx\n\t"
               "int $0x15\n\t"
               "pushfl\n\t"
               "popl %0\n\t"
               "movl %%ebx, %1\n\t"
               : "=r"(eflags), "+m"(*ebx)
               : "c"(20), "d"(0x534d4150), "a"(0xe820), "D"((unsigned long)dst)
               : "memory", "%ebx");
  if (eflags & EFLAGS_CF) {
    return -1;
  }
  return 0;
}

#endif
