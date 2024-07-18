#include <kernel/js/engine.h>
#include <serial/serial.h>
#include <string.h>

#define JS_BUF_SIZE 1024 * 1024

char JS_BUF[JS_BUF_SIZE];


static jsval_t cprint(struct js *js, jsval_t *args, int nargs) {
  for (int i = 0; i < nargs; i++) {
    const char *s = js_getstr(js, args[i], (void *)0);
    serial_puts(SERIAL_DEFAULT_SERIAL_PORT, s);
  }
  return js_mknull();
}

void __attribute__((section("entry"))) _start() {
  serial_puts(SERIAL_DEFAULT_SERIAL_PORT, "we are in kernel now!\n");
  const char *js_code = "cprint(\"hello world from javascript\");";

  struct js* instance = js_create(JS_BUF, sizeof(JS_BUF));
  js_set(instance, js_glob(instance), "cprint", js_mkfun(cprint));
  jsval_t result = js_eval(instance, js_code, strlen(js_code));

  if (js_type(result) == JS_ERR) {
    serial_puts(SERIAL_DEFAULT_SERIAL_PORT, "eval java script error!\n");
  }

  while (1) {
    asm volatile("nop\n\t");
  }
}
