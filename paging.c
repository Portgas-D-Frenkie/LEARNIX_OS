#include "paging.h"
#include "vga.h"

/* Page Directory y Page Table como arrays estáticos
 * El linker los coloca automáticamente después del kernel */
static uint32_t page_directory[1024] __attribute__((aligned(4096)));
static uint32_t page_table[1024]     __attribute__((aligned(4096)));

void paging_init() {
    uint32_t i;

    /* Limpiar Page Directory */
    for (i = 0; i < 1024; i++) {
        page_directory[i] = 0x00000002;
    }

    /* Identity map primeros 4MB */
    for (i = 0; i < 1024; i++) {
        page_table[i] = (i * 0x1000) | 3;
    }

    /* PD[0] apunta a nuestra page table */
    page_directory[0] = ((uint32_t)page_table) | 3;

    /* Cargar CR3 */
    __asm__ volatile (
        "mov %0, %%cr3"
        : : "r"((uint32_t)page_directory) : "memory"
    );

    /* Activar paginación bit PG en CR0 */
    __asm__ volatile (
        "mov %%cr0, %%eax\n"
        "or $0x80000000, %%eax\n"
        "mov %%eax, %%cr0\n"
        : : : "eax", "memory"
    );
}

void paging_map(uint32_t virt, uint32_t phys, uint32_t flags) {
    uint32_t pt_idx = (virt >> 12) & 0x3FF;
    page_table[pt_idx] = phys | (flags & 0xFFF) | 1;
}

void paging_print_info() {
    uint32_t cr0, cr3;
    __asm__ volatile ("mov %%cr0, %0" : "=r"(cr0));
    __asm__ volatile ("mov %%cr3, %0" : "=r"(cr3));

    vga_set_color(VGA_LIGHT_CYAN, VGA_BLACK);
    vga_print("[VMM] Page Directory: 0x");
    vga_print_hex((uint32_t)page_directory);
    vga_print("\n");

    vga_print("[VMM] Page Table:     0x");
    vga_print_hex((uint32_t)page_table);
    vga_print("\n");

    vga_print("[VMM] CR3 = 0x");
    vga_print_hex(cr3);
    vga_print("\n");

    if (cr0 & 0x80000000) {
        vga_set_color(VGA_LIGHT_GREEN, VGA_BLACK);
        vga_print("[VMM] Paginacion ACTIVA!\n");
        vga_print("[VMM] VMM listo!\n");
    } else {
        vga_set_color(VGA_LIGHT_RED, VGA_BLACK);
        vga_print("[VMM] ERROR: Paginacion no activa!\n");
    }
}
