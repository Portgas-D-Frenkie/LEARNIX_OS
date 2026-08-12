#include "elf.h"
#include "fat16.h"
#include "process.h"
#include "vga.h"
#include <stdint.h>

/* Buffer estatico para el ELF — maximo 64 KB */
static uint8_t elf_buffer[65536];

/* -------------------------------------------------------
 * elf_verify()
 * Verifica que el ELF Header es valido para x86 32-bit
 * Retorna 0 si es valido, -1 si no
 * ------------------------------------------------------- */
int elf_verify(elf32_hdr_t *hdr) {
    /* Verificar magic: 0x7F 'E' 'L' 'F' */
    if (hdr->e_ident[0] != 0x7F ||
        hdr->e_ident[1] != 'E'  ||
        hdr->e_ident[2] != 'L'  ||
        hdr->e_ident[3] != 'F') {
        vga_set_color(VGA_LIGHT_RED, VGA_BLACK);
        vga_print("[ELF] ERROR: Magic bytes invalidos\n");
        return -1;
    }

    /* Verificar que es de 32 bits */
    if (hdr->e_ident[4] != 1) {
        vga_set_color(VGA_LIGHT_RED, VGA_BLACK);
        vga_print("[ELF] ERROR: Solo se soporta ELF 32-bit\n");
        return -1;
    }

    /* Verificar que es little endian */
    if (hdr->e_ident[5] != 1) {
        vga_set_color(VGA_LIGHT_RED, VGA_BLACK);
        vga_print("[ELF] ERROR: Solo se soporta little endian\n");
        return -1;
    }

    /* Verificar que es un ejecutable */
    if (hdr->e_type != ET_EXEC) {
        vga_set_color(VGA_LIGHT_RED, VGA_BLACK);
        vga_print("[ELF] ERROR: No es un ejecutable (ET_EXEC)\n");
        return -1;
    }

    /* Verificar arquitectura x86 */
    if (hdr->e_machine != EM_386) {
        vga_set_color(VGA_LIGHT_RED, VGA_BLACK);
        vga_print("[ELF] ERROR: Solo se soporta x86 (EM_386)\n");
        return -1;
    }

    return 0;  /* ELF valido */
}

/* -------------------------------------------------------
 * elf_print_info()
 * Muestra informacion del ELF Header en pantalla
 * ------------------------------------------------------- */
void elf_print_info(elf32_hdr_t *hdr) {
    vga_set_color(VGA_LIGHT_CYAN, VGA_BLACK);
    vga_print("[ELF] Informacion del ejecutable:\n");

    vga_set_color(VGA_WHITE, VGA_BLACK);
    vga_print("[ELF] Entry point: 0x");
    vga_print_hex(hdr->e_entry);
    vga_print("\n");

    vga_print("[ELF] Program Headers: ");
    vga_print_int(hdr->e_phnum);
    vga_print("\n");

    vga_print("[ELF] Section Headers: ");
    vga_print_int(hdr->e_shnum);
    vga_print("\n");
}

/* -------------------------------------------------------
 * elf_load()
 * Carga un ejecutable ELF desde FAT16 y crea un proceso
 *
 * filename → nombre del archivo en FAT16 (formato 8+3)
 * Retorna 0 si OK, -1 si error
 * ------------------------------------------------------- */
int elf_load(const char *filename) {
    vga_set_color(VGA_LIGHT_CYAN, VGA_BLACK);
    vga_print("[ELF] Cargando ejecutable: ");
    vga_print(filename);
    vga_print("\n");

    /* ── Paso 1: Leer el ELF desde FAT16 ─────────────── */
    int bytes = fat16_read_file(filename, elf_buffer,
                                sizeof(elf_buffer));
    if (bytes < 0) {
        vga_set_color(VGA_LIGHT_RED, VGA_BLACK);
        vga_print("[ELF] ERROR: No se encontro el archivo\n");
        return -1;
    }

    vga_set_color(VGA_WHITE, VGA_BLACK);
    vga_print("[ELF] Bytes leidos: ");
    vga_print_int(bytes);
    vga_print("\n");

    /* ── Paso 2: Verificar ELF Header ────────────────── */
    elf32_hdr_t *hdr = (elf32_hdr_t *) elf_buffer;
    if (elf_verify(hdr) < 0) return -1;

    elf_print_info(hdr);

    /* ── Paso 3: Cargar segmentos PT_LOAD ────────────── */
    elf32_phdr_t *phdrs =
        (elf32_phdr_t *)(elf_buffer + hdr->e_phoff);

    int segmentos_cargados = 0;

    for (int i = 0; i < hdr->e_phnum; i++) {
        /* Solo nos interesan los segmentos PT_LOAD */
        if (phdrs[i].p_type != PT_LOAD) continue;

        uint8_t *dest = (uint8_t *) phdrs[i].p_vaddr;
        uint8_t *src  = elf_buffer + phdrs[i].p_offset;

        vga_set_color(VGA_WHITE, VGA_BLACK);
        vga_print("[ELF] Segmento ");
        vga_print_int(i);
        vga_print(": 0x");
        vga_print_hex(phdrs[i].p_vaddr);
        vga_print(" size=");
        vga_print_int(phdrs[i].p_filesz);
        vga_print(" bytes\n");

        /* Copiar datos del archivo a la direccion virtual */
        for (uint32_t b = 0; b < phdrs[i].p_filesz; b++)
            dest[b] = src[b];

        /* Inicializar .bss con ceros
         * (memsz > filesz cuando hay variables no iniciadas) */
        for (uint32_t b = phdrs[i].p_filesz;
             b < phdrs[i].p_memsz; b++)
            dest[b] = 0;

        segmentos_cargados++;
    }

    vga_set_color(VGA_WHITE, VGA_BLACK);
    vga_print("[ELF] Segmentos cargados: ");
    vga_print_int(segmentos_cargados);
    vga_print("\n");

    vga_set_color(VGA_YELLOW, VGA_BLACK);
    // vga_print("[ELF] Llamando process_create...\n");
    void (*entry)() = (void(*)()) hdr->e_entry;

    /* PRUEBA: ejecutar directamente, sin scheduler */
    vga_set_color(VGA_YELLOW, VGA_BLACK);
    vga_print("[ELF] Saltando al entry point directamente...\n");
    entry();
    vga_print("[ELF] El programa retorno\n");

    process_t *proc = process_create(entry, filename, PRIORITY_NORMAL);

    // vga_print("[ELF] process_create retorno\n");

    if (!proc) {
        vga_set_color(VGA_LIGHT_RED, VGA_BLACK);
        vga_print("[ELF] ERROR: kmalloc fallo (sin memoria)\n");
        return -1;
    }

    vga_set_color(VGA_LIGHT_GREEN, VGA_BLACK);
    vga_print("[ELF] Proceso creado PID=");
    vga_print_int(proc->pid);
    vga_print("\n");
    return 0;
}
