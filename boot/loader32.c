asm(".code32");

#include <boot/link.h>
#include <boot/loader.h>

__attribute__((section("header.entry32"))) void loader32() {
  // reset all registers
  asm volatile("mov %%ax, %%ds\n\t"
               "mov %%ax, %%es\n\t"
               "mov %%ax, %%fs\n\t"
               "mov %%ax, %%gs\n\t"
               "mov %%ax, %%ss\n\t" ::"a"(BOOT_INIT_DS));

  //  复位栈寄存器
  asm volatile("mov %%eax, %%esp\n\t"
               "mov %%eax, %%ebp\n\t" ::"a"(PM_STACK_TOP)
               : "esp", "ebp");
  pm_start();
}
