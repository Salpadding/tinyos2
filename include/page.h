#ifndef _PAGE_H
#define _PAGE_H

#define PAGE_SIZE 4096

#include <size.h>

#ifdef __ASM__
#define __AC(x, y) (x)
#else
#define __AC(x, y) (x##y)
#endif

#define PAGE_OFFSET __AC(0xffff800000000000, UL)

#define __pa(x) (((unsigned long)x) - PAGE_OFFSET)
#define __va(x) (((unsigned long)x) + PAGE_OFFSET)

#endif
