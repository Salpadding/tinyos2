asm(".code32");
#include <boot/link.h>
#include <cpu.h>
#include <serial/serial.h>
#include <size.h>
#include <x86/printf.h>
#include <8259a.h>

static int printf(const char *fmt, ...) {
  char printf_buf[128];
  va_list args;
  int printed;

  va_start(args, fmt);
  printed = x86_print_vsprintf(printf_buf, fmt, args);
  va_end(args);

  serial_puts_n(SERIAL_DEFAULT_SERIAL_PORT, printf_buf, printed);

  return printed;
}

extern int _text_end;

#define SERIAL_PORT SERIAL_DEFAULT_SERIAL_PORT

#define CPU_ID_FN 0x80000000

void _pm_start();
void setup_pae();

static void memcpy(void *dst, void *src, unsigned long count) {
  asm volatile("cld\n\t"
               "rep movsb\n\t" ::"D"(dst),
               "S"(src), "c"(count));
}

#define RESET_SEG(seg)                                                         \
  asm volatile("mov %%ax, %%ds\n\t"                                            \
               "mov %%ax, %%es\n\t"                                            \
               "mov %%ax, %%fs\n\t"                                            \
               "mov %%ax, %%gs\n\t"                                            \
               "mov %%ax, %%ss\n\t" ::"a"(seg))

uint64_t gdt_64[3] __attribute__((aligned(0x10))) = {
    [0] = 0,
    [1] = 0x0020980000000000,
    [2] = 0x0000920000000000,
};

typedef struct {
  uint16_t length;
  uint64_t addr;
} __attribute__((packed)) gdt_ptr_t;

gdt_ptr_t gdt_64_ptr
    __attribute__((aligned(0x10))) = {.length = sizeof(gdt_64) - 1, .addr = 0};

void pm_start() {
  RESET_SEG(BOOT_INIT_DS);
  _pm_start();
}

#define SPIN                                                                   \
  while (1) {                                                                  \
    asm volatile("nop\n\t");                                                   \
  }

#define TEST_CPU_FEATURE(x, y, z)                                              \
  if (!(cpu_features & (x))) {                                                 \
    printf(z);                                                                 \
    SPIN;                                                                      \
  }                                                                            \
  printf(y);


void __attribute__((noinline)) _pm_start() {
  gdt_64_ptr.addr = (uint64_t)&gdt_64;
  printf("we are now in protect mode\n");

  unsigned long i;
  char *src = (void *)LOADER_ENTRY;
  char *dst = (void *)0x200000;

  if (!cpuh_longmode()) {
    printf("long mode is not supported\n");
    while (1)
      ;
  }
  printf("detect long mode support, go on setup page table\n");

  unsigned long cpu_features = cpuh_features();
  printf("cpu features = %x\n", cpuh_features);

  // try to enable sse and sse2
  TEST_CPU_FEATURE(1 << 25, "SSE supported\n", "SSE not supported\n");
  TEST_CPU_FEATURE(1 << 26, "SSE2 supported\n", "SSE2 not supported\n");

  cpuh_enable_sse();

  memcpy((void *)KERNEL_ENTRY, (void *)KERNEL_TMP, KERNEL_SECS * SEC_SIZE);
  // 开启 pae
  setup_pae();

  printf("ready to jump to kernel code\n");

  asm volatile(".globl enter_64\nenter_64:\n\t");
  // 准备跳到 0x100000 执行 64 位代码
  asm volatile("lgdt %0\n\t"
               "ljmp %1, %2" ::"m"(*&gdt_64_ptr),
               "i"(BOOT_INIT_CS), "i"(KERNEL_ENTRY));
}

// identity mapping first 2MB
void setup_pae() {
  uint64_t i;
#define PAGE_TABLE_ENTRYS INIT_PAGE_TABLE_ENTRYS
  uint64_t *pud = (void *)INIT_PAGETABLE;
  uint64_t *pmd = (void *)(INIT_PAGETABLE + PAGE_SIZE);
  uint64_t *pgd = (void *)(INIT_PAGETABLE + PAGE_SIZE * 2);
  uint64_t *pt0 = (void *)(INIT_PAGETABLE + PAGE_SIZE * 3);
  uint64_t *pt1 = (void *)(INIT_PAGETABLE + PAGE_SIZE * 4);

  // clear all
  for (i = 0; i < PAGE_TABLE_ENTRYS * 8; i++) {
    pud[i] = 0;
  }

  asm volatile(".globl fill_init_page_table\n fill_init_page_table:\n\t");
  pud[0] = ((uint64_t)pmd) | 7;
  pud[256] = pud[0];
  pmd[0] = ((uint64_t)pgd) | 7;
  pgd[0] = ((uint64_t)pt0) | 7;
  pgd[1] = ((uint64_t)pt1) | 7;

  for (i = 0; i < PAGE_TABLE_ENTRYS * 2; i++) {
    pt0[i] = (i << 12) | 7;
  }

  unsigned long tmp;
  // cr3
  asm volatile("movl %0, %%cr3\n\t" ::"r"((unsigned long)pud) :);

  RESET_SEG(BOOT_INIT_DS);

  // pae
  asm volatile("mov %%cr4, %0\n\t"
               "or  %1, %0\n\t"
               "mov %0, %%cr4\n\t"
               : "=r"(tmp)
               : "i"(1 << 5)
               :);

  serial_puts(SERIAL_PORT, "pae setup done\n");

  // enable long mode
  asm volatile("rdmsr\n\t"
               "or %1, %%eax\n\t"
               "wrmsr\n\t" ::"c"(0xc0000080),
               "i"(1 << 8)
               :);

  serial_puts(SERIAL_PORT, "long mode enabled\n");

  // enable paging
  asm volatile("movl %%cr0, %0\n\t"
               "or %1, %0\n\t"
               "movl %0, %%cr0\n\t"
               : "=r"(tmp)
               : "i"(1 << 31));

  serial_puts(SERIAL_PORT,
              "paging enabled, we are now in 32-bit compatibility submode\n");
}
