#ifndef PAGING_H
#define PAGING_H

#include <stdint.h>

#define PAGE_PRESENT    0x1
#define PAGE_WRITABLE   0x2
#define PAGE_USER       0x4
#define PAGE_SIZE       4096

void paging_init();
void paging_map(uint32_t virt, uint32_t phys, uint32_t flags);
void paging_print_info();

#endif
