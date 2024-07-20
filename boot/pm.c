asm(".code32");

#include <8259a.h>
#include <boot/link.h>
#include <cpu.h>
#include <serial/serial.h>
#include <size.h>
#include <x86/printf.h>

typedef uint16_t u16;
typedef uint32_t u32;

static void _set_gate(void *gate_addr, char type, char dpl, void *addr,
                      short cs) {
  ((u16 *)gate_addr)[0] = (u16)((u32)addr & 0xffff);

  ((u16 *)gate_addr)[1] = (u16)cs;
  ((u16 *)gate_addr)[2] =
      (((u16)type) << 8) | (((u16)dpl) << 13) | ((u16)0x8000);

  ((u16 *)gate_addr)[3] = (u16)((u32)addr >> 16);
}

#define set_intr_gate(n, addr, cs) _set_gate((void *)(&idt[n]), 14, 0, addr, cs)

#define set_trap_gate(n, addr, cs) _set_gate(&idt[n], 15, 0, addr, cs)

#define set_system_gate(n, addr, cs) _set_gate(&idt[n], 15, 3, addr, cs)

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

extern int _etext;

#define SERIAL_PORT SERIAL_DEFAULT_SERIAL_PORT

void setup_pae();

static void memcpy(void *dst, void *src, unsigned long count) {
  asm volatile("cld\n\t"
               "rep movsb\n\t" ::"D"(dst),
               "S"(src), "c"(count));
}

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

__attribute__((aligned(0x10))) uint64_t idt[256];

struct idt_ptr_t {
  uint16_t length;
  uint32_t addr;
} __attribute__((packed));

struct idt_ptr_t idt_ptr = {
    .length = sizeof(idt) - 1,
    .addr = 0,
};

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

void __attribute__((noinline)) pm_start() {
  // read current cs
  unsigned long cs;
  asm volatile("\tpushl %%cs\n\tpopl %0\n" : "=m"(*&cs) : :);
  printf("current cs = %x\n", cs);

  idt_ptr.addr = (uint32_t)(&idt[0]);
  gdt_64_ptr.addr = (uint64_t)&gdt_64;
  printf("we are now in protect mode\n");

  // since we are in protect mode, we can init 8259 pic
  // we also mark all interrupt, since the idt have not setup yet
  _8259A_init(1, 0b11111011, 0b11111111);

  asm volatile("sti\n");

  unsigned long i;
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

  // enable sse
  cpuh_enable_sse();

  // 开启 pae
  setup_pae();

  printf("ready to jump to kernel code\n");

  // 准备跳到 0x100000 执行 64 位代码
#ifdef LOAD_KERNEL
  asm volatile("cli");
  asm volatile("lgdt %0\n\t"
               "ljmp %1, %2" ::"m"(*&gdt_64_ptr),
               "i"(BOOT_INIT_CS), "i"(KERNEL_ENTRY));
#else
  while (1) {
  }
#endif
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
