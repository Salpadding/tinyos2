#include <gdt.h>
#include <size.h>
#include <page.h>

#define GDT_LEN 16
#define IDT_LEN 255

#define ALIGN __attribute__((aligned(0x10)))

uint64_t ALIGN gdt[GDT_LEN] = {
    0,
    0x0020980000000000, /*1	KERNEL	Code	64-bit	Segment	08*/
    0x0000920000000000, /*2	KERNEL	Data	64-bit	Segment	10*/
    0x0000000000000000, /*3	USER	Code	32-bit	Segment 18*/
    0x0000000000000000, /*4	USER	Data	32-bit	Segment 20*/
    0x0020f80000000000, /*5	USER	Code	64-bit	Segment	28*/
    0x0000f20000000000, /*6	USER	Data	64-bit	Segment	30*/
    0x00cf9a000000ffff, /*7	KERNEL	Code	32-bit	Segment	38*/
    0x00cf92000000ffff, /*8	KERNEL	Data	32-bit	Segment	40*/
};

uint64_t ALIGN idt[IDT_LEN] = {0};

struct gdt_ptr_t ALIGN gdt_ptr = {
    .length = sizeof(gdt) - 1,
    .addr = ((uint64_t)&gdt),
};

struct gdt_ptr_t ALIGN idt_ptr = {
    .length = sizeof(idt) - 1,
    .addr = (uint64_t)&idt,
};
