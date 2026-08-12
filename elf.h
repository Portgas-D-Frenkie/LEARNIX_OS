#ifndef ELF_H
#define ELF_H

#include <stdint.h>

/* ── Tipos de archivo ELF (e_type) ─────────────────── */
#define ET_EXEC    2   /* Ejecutable                    */

/* ── Arquitecturas (e_machine) ─────────────────────── */
#define EM_386     3   /* x86 32-bit                    */

/* ── Tipos de segmento (p_type) ────────────────────── */
#define PT_NULL    0   /* Ignorar                       */
#define PT_LOAD    1   /* Segmento a cargar en memoria  */

/* ── Flags de segmento (p_flags) ───────────────────── */
#define PF_X    0x01   /* Ejecutable                    */
#define PF_W    0x02   /* Escribible                    */
#define PF_R    0x04   /* Legible                       */

/* ── ELF Header 32-bit (52 bytes) ──────────────────── */
typedef struct {
    uint8_t  e_ident[16];   /* Magic + clase + endian  */
    uint16_t e_type;        /* Tipo de archivo         */
    uint16_t e_machine;     /* Arquitectura            */
    uint32_t e_version;     /* Version ELF             */
    uint32_t e_entry;       /* Punto de entrada        */
    uint32_t e_phoff;       /* Offset Program Headers  */
    uint32_t e_shoff;       /* Offset Section Headers  */
    uint32_t e_flags;       /* Flags                   */
    uint16_t e_ehsize;      /* Tamano ELF Header       */
    uint16_t e_phentsize;   /* Tamano Program Header   */
    uint16_t e_phnum;       /* Num. Program Headers    */
    uint16_t e_shentsize;   /* Tamano Section Header   */
    uint16_t e_shnum;       /* Num. Section Headers    */
    uint16_t e_shstrndx;    /* Indice tabla nombres    */
} __attribute__((packed)) elf32_hdr_t;

/* ── Program Header 32-bit (32 bytes) ──────────────── */
typedef struct {
    uint32_t p_type;    /* Tipo: PT_LOAD etc.          */
    uint32_t p_offset;  /* Offset en el archivo        */
    uint32_t p_vaddr;   /* Direccion virtual destino   */
    uint32_t p_paddr;   /* Direccion fisica (ignorada) */
    uint32_t p_filesz;  /* Bytes a copiar del archivo  */
    uint32_t p_memsz;   /* Bytes en memoria (>filesz si .bss) */
    uint32_t p_flags;   /* Permisos R/W/X              */
    uint32_t p_align;   /* Alineacion                  */
} __attribute__((packed)) elf32_phdr_t;

/* ── Funciones publicas ─────────────────────────────── */
int  elf_verify(elf32_hdr_t *hdr);
int  elf_load(const char *filename);
void elf_print_info(elf32_hdr_t *hdr);

#endif
