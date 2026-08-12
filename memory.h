#ifndef MEMORY_H
#define MEMORY_H
#include <stdint.h>

/* Direcciones del mapa E820 (bootloader casero) */
#define MEMORY_MAP_ADDR  0x500
#define MEMORY_MAP_COUNT 0x4F0

/* Tipos de región */
#define MEM_TYPE_USABLE   1
#define MEM_TYPE_RESERVED 2
#define MEM_TYPE_ACPI     3
#define MEM_TYPE_NVS      4
#define MEM_TYPE_BAD      5

/* Estructura E820 (bootloader casero) */
typedef struct {
    uint64_t base;
    uint64_t length;
    uint32_t type;
    uint32_t acpi;
} __attribute__((packed)) memory_region_t;

/* Estructura Multiboot mmap entry */
typedef struct {
    uint32_t size;
    uint64_t base;
    uint64_t length;
    uint32_t type;
} __attribute__((packed)) multiboot_mmap_entry_t;

/* Estructura Multiboot info */
typedef struct {
    uint32_t flags;
    uint32_t mem_lower;
    uint32_t mem_upper;
    uint32_t boot_device;
    uint32_t cmdline;
    uint32_t mods_count;
    uint32_t mods_addr;
    uint32_t syms[4];
    uint32_t mmap_length;
    uint32_t mmap_addr;
} __attribute__((packed)) multiboot_info_t;

/* Funciones */
void     memory_init();
void     memory_init_multiboot(uint32_t magic, multiboot_info_t *mbi);
uint32_t memory_get_total();
void     memory_print_map();

#endif
