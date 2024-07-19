#include <gdt.h>
#include <kernel/js/engine.h>
#include <kernel/script.h>
#include <page.h>
#include <serial/serial.h>
#include <stack.h>
#include <string.h>
#include <x86/printf.h>

#define RESET_SEG(seg)                                                         \
  asm volatile("mov %%ax, %%ds\n\t"                                            \
               "mov %%ax, %%es\n\t"                                            \
               "mov %%ax, %%fs\n\t"                                            \
               "mov %%ax, %%gs\n\t"                                            \
               "mov %%ax, %%ss\n\t" ::"a"(seg))

#define JS_BUF_SIZE 1024 * 1024

__attribute__((aligned(PAGE_SIZE))) char JS_BUF[JS_BUF_SIZE];

static int printf(const char *fmt, ...) {
  char printf_buf[1024];
  va_list args;
  int printed;

  va_start(args, fmt);
  printed = x86_print_vsprintf(printf_buf, fmt, args);
  va_end(args);

  serial_puts(SERIAL_DEFAULT_SERIAL_PORT, printf_buf);

  return printed;
}

// Prints all arguments, one by one, delimit by space
static jsval_t js_print(struct js *js, jsval_t *args, int nargs) {
  size_t size;
  int i;
  const char *space;
  for (i = 0; i < nargs; i++) {
    space = i == 0 ? "" : " ";
    if (js_type(args[i]) == JS_STR) {
      printf("call js_getstr\n");
      const char *s = js_getstr(js, args[i], &size);
      printf("%s%.*s", space, size, s);
    } else {
      printf("%s%s", space, js_str(js, args[i]));
    }
  }
  printf("\n"); // Finish by newline
  return js_mkundef();
}

void __attribute__((section("entry"))) _start() {
  unsigned long tmp;

  // 加载高地址的 gdt ptr
  asm volatile("lgdt (%0)" ::"r"(&gdt_ptr) :);

  // 重置 cs, rip
  asm volatile("leaq 1f(%%rip), %0\n\t"
               "add %1, %0\n\t"
               "push %2\n\t"
               "push %0\n\t"
               "lretq \n\t"
               "1: \n\t"
               : "=a"(tmp)
               : "b"(PAGE_OFFSET), "i"(KERNEL_CS));

  RESET_SEG(KERNEL_DS);

  // 重置栈指针
  asm volatile("mov %0, %%rsp\n\t"
               "mov %%rsp, %%rbp\n\t" ::"a"(
                   &init_task_union.stack[STACK_SIZE / sizeof(unsigned long)]));

  printf("we are in kernel now!\n");

  // create js runtime
  struct js *instance = js_create(JS_BUF, sizeof(JS_BUF));

  printf("register print function at %p\n", js_print);
  js_set(instance, js_glob(instance), "print", js_mkfun(js_print));

  printf("eval js code\n");

  jsval_t result =
      js_eval(instance, (const char *)&script_init_js[0], script_init_js_len);

  if (js_type(result) == JS_ERR) {
    printf("eval java script error %s\n", js_str(instance, result));
  }

  while (1) {
    asm volatile("nop\n\t");
  }
}
