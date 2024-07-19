#ifndef _BOOT_LINK_H
#define _BOOT_LINK_H

#define BOOT_ENTRY 0x7c00
#define LOADER_ENTRY 0x500

#define LOADER_SECS 32
#define KERNEL_SECS 896

#define KERNEL_TMP 0x10000
#define KERNEL_ENTRY 0x100000

#define SEC_SIZE 0x200

#define E820_MAP_ADDR 0x8000
#define E820_MAP_LEN (void*)(0x7e00)


#define BOOT_INIT_CS 0x08
#define BOOT_INIT_DS 0x10

#define PAGE_SIZE 4096
#define INIT_PAGETABLE (0x10000)
#define INIT_PAGE_TABLE_ENTRYS (PAGE_SIZE / sizeof(uint64_t))


#endif
