#ifndef _SERIAL_SERIAL_H
#define _SERIAL_SERIAL_H

#ifdef SECTION_NAME
#define ATTRIBUTES __attribute__((section(SECTION_NAME)))
#endif

#ifndef ATTRIBUTES
#define ATTRIBUTES
#endif

#define SERIAL_DEFAULT_SERIAL_PORT 0x3f8

#include <asm/io.h>

#define INIT_SERIAL(SERIAL_PORT)                                               \
  outb_p(0x80, SERIAL_PORT + 3);                                               \
  outb_p(0x30, SERIAL_PORT);                                                   \
  outb_p(0x00, SERIAL_PORT + 1);                                               \
  outb_p(0x03, SERIAL_PORT + 3);                                               \
  outb_p(0x0b, SERIAL_PORT + 4);                                               \
  outb_p(0x0d, SERIAL_PORT + 1);

ATTRIBUTES
static void serial_putc(int port, char c) {
  unsigned int ax = 0;

  while (!(ax & 0xff)) {
    asm volatile("inb %%dx,%%al" : "=a"(ax) : "d"(port + 5) :);
  }
  outb_p(c, port);
}

ATTRIBUTES
static void serial_puts(int port, const char *s) {
  while (*s) {
    serial_putc(port, *(s++));
  }
}

ATTRIBUTES
static void serial_puts_n(int port, const char *s, int n) {
  while (*s && n) {
    serial_putc(port, *(s++));
    n--;
  }
}
#endif
