#ifndef _CPU_H
#define _CPU_H

// 这里 gcc 不知道 cpuid 会修改 edx, 所以把 edx 加入 clobber list
// 不这么做的话, 有可能 gcc 会使用 edx 作为临时寄存器
static unsigned long cpuh_cpuid(unsigned long eax, unsigned long *edx) {
  asm volatile("cpuid\n\t"
               "movl %%edx, %1\n\t"
               : "=a"(eax), "=m"(*edx)
               : "0"(eax)
               : "ebx", "ecx", "edx");
  return eax;
}

static int cpuh_longmode() {
  unsigned long edx;
  return cpuh_cpuid(0x80000000, &edx) >= 0x80000001;
}

static unsigned long cpuh_features() {
  unsigned long edx;
  cpuh_cpuid(0x1, &edx);
  return edx;
}


static void cpuh_enable_sse() {
  unsigned int cr4;

  asm volatile("mov %%cr4, %0\n" : "=r"(cr4));

  // Set OSFXSR (bit 9) and OSXMMEXCPT (bit 10)
  cr4 |= (1 << 9) | (1 << 10);

  asm volatile("mov %0, %%cr4\n" : : "r"(cr4));
}

#endif
