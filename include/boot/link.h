#ifndef _BOOT_LINK_H
#define _BOOT_LINK_H

#define SECTOR_SIZE 512
#define BOOT_ENTRY 0x7c00
#define BOOT_SP 0xfff0

#define HEADER_ENTRY 0x1000
#define HEADER_MAX_SIZE 0x4000
#define HEADER_MAX_SECTORS (HEADER_MAX_SIZE / SECTOR_SIZE)

#define PM_ENTRY 0x10000
#define PM_SIZE  0x50000
#define PM_SECTORS (PM_SIZE / SECTOR_SIZE)
#define PM_STACK_TOP 0x7ff00

#define LOADER_SECTORS 768

#if LOADER_SECTORS < (HEADER_SECTORS + PM_SECTORS)
#error "sectors over flow"
#endif

#define LOADER_FIRST_LBA 1024

#define E820_MAP_LEN (void *)(0x7e00)

#define BOOT_INIT_CS 0x08
#define BOOT_INIT_DS 0x10

#define PAGE_SIZE 4096

#define INIT_PAGETABLE (0x10000)
#define INIT_PAGE_TABLE_ENTRYS (PAGE_SIZE / sizeof(uint64_t))

#endif
