#ifndef _LOADER_H
#define _LOADER_H

#include <boot/link.h>
#include <size.h>

extern void pm_start();
extern void loader32();

extern char e820_table[PAGE_SIZE];
extern uint16_t e820_count;

#endif
