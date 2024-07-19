#ifndef _GDT_H
#define _GDT_H

#include <size.h>
#include <page.h>

extern uint64_t gdt[];

struct __attribute__((packed)) gdt_ptr_t {
  uint16_t length;
  uint64_t addr;
};

extern struct gdt_ptr_t gdt_ptr;
extern struct gdt_ptr_t idt_ptr;


#define KERNEL_CS 0x08
#define KERNEL_DS 0x10

#define STACK_SIZE (PAGE_SIZE << 3)

#endif
