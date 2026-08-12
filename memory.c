#include "memory.h"
#include "vga.h"
#include "memory.h"
#include "vga.h"

static uint32_t total_memory = 0;
static uint32_t region_count = 0;
static multiboot_info_t *mbi_ptr = 0;

void memory_init_multiboot(uint32_t magic, multiboot_info_t *mbi) {
    if (magic != 0x2BADB002) return;
    mbi_ptr = mbi;
    if (mbi->flags & (1 << 0)) {
        total_memory = (mbi->mem_upper + 1024) * 1024;
    }
}

void memory_init() {
    memory_region_t *regions = (memory_region_t *) MEMORY_MAP_ADDR;
    region_count = *((uint32_t *) MEMORY_MAP_COUNT);
    total_memory = 0;
    for (uint32_t i = 0; i < region_count; i++) {
        if (regions[i].type == MEM_TYPE_USABLE) {
            total_memory += (uint32_t) regions[i].length;
        }
    }
}

uint32_t memory_get_total() {
    return total_memory;
}

void memory_print_map() {
    static const char *type_names[] = {
        "Unknown", "Usable", "Reserved", "ACPI", "NVS", "Bad"
    };

    vga_set_color(VGA_LIGHT_CYAN, VGA_BLACK);
    vga_print("[MEM] Detectando memoria RAM...\n");

    if (mbi_ptr && (mbi_ptr->flags & (1 << 6))) {
        multiboot_mmap_entry_t *entry =
            (multiboot_mmap_entry_t *) mbi_ptr->mmap_addr;
        multiboot_mmap_entry_t *end =
            (multiboot_mmap_entry_t *)
            (mbi_ptr->mmap_addr + mbi_ptr->mmap_length);
        int i = 0;
        while (entry < end) {
            vga_set_color(VGA_WHITE, VGA_BLACK);
            vga_print("[MEM] Region ");
            vga_print_int(i);
            vga_print(": base=0x");
            vga_print_hex((uint32_t)entry->base);
            vga_print(" size=");
            vga_print_int((uint32_t)(entry->length / 1024));
            vga_print(" KB tipo: ");
            if (entry->type == MEM_TYPE_USABLE)
                vga_set_color(VGA_LIGHT_GREEN, VGA_BLACK);
            else
                vga_set_color(VGA_LIGHT_RED, VGA_BLACK);
            if (entry->type <= 5)
                vga_print(type_names[entry->type]);
            else
                vga_print("Unknown");
            vga_print("\n");
            entry = (multiboot_mmap_entry_t *)
                ((uint32_t)entry + entry->size + sizeof(uint32_t));
            i++;
        }
    } else {
        memory_region_t *regions = (memory_region_t *) MEMORY_MAP_ADDR;
        for (uint32_t i = 0; i < region_count; i++) {
            vga_set_color(VGA_WHITE, VGA_BLACK);
            vga_print("[MEM] Region ");
            vga_print_int(i);
            vga_print(": base=0x");
            vga_print_hex((uint32_t)regions[i].base);
            vga_print(" size=");
            vga_print_int((uint32_t)regions[i].length / 1024);
            vga_print(" KB tipo: ");
            if (regions[i].type == MEM_TYPE_USABLE)
                vga_set_color(VGA_LIGHT_GREEN, VGA_BLACK);
            else
                vga_set_color(VGA_LIGHT_RED, VGA_BLACK);
            if (regions[i].type <= 5)
                vga_print(type_names[regions[i].type]);
            else
                vga_print("Unknown");
            vga_print("\n");
        }
    }

    vga_set_color(VGA_YELLOW, VGA_BLACK);
    vga_print("[MEM] Total RAM: ");
    vga_print_int(total_memory / (1024 * 1024));
    vga_print(" MB\n");
}
