asm(".code32");

#include <asm/io.h>

#define outb(b,p) outb_p(b, p)
#include <8259a.c>
