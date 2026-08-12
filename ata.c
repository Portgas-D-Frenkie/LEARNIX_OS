#include "ata.h"
#include "vga.h"
#include <stdint.h>

/* -------------------------------------------------------
 * Funciones de I/O de bajo nivel
 * ------------------------------------------------------- */
static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t val;
    __asm__ volatile ("inb %1, %0" : "=a"(val) : "Nd"(port));
    return val;
}

static inline uint16_t inw(uint16_t port) {
    uint16_t val;
    __asm__ volatile ("inw %1, %0" : "=a"(val) : "Nd"(port));
    return val;
}

static inline void outw(uint16_t port, uint16_t val) {
    __asm__ volatile ("outw %0, %1" : : "a"(val), "Nd"(port));
}

/* -------------------------------------------------------
 * ata_wait_ready()
 * Espera hasta que el disco no esté ocupado
 * ------------------------------------------------------- */
static int ata_wait_ready() {
    uint32_t timeout = 100000;
    while (timeout--) {
        uint8_t status = inb(ATA_PRIMARY_STATUS);
        if (status & ATA_STATUS_ERR) return -1;  /* Error */
        if (!(status & ATA_STATUS_BSY) &&
             (status & ATA_STATUS_DRDY)) return 0; /* Listo */
    }
    return -1;  /* Timeout */
}

/* -------------------------------------------------------
 * ata_wait_drq()
 * Espera hasta que el disco tenga datos listos
 * ------------------------------------------------------- */
static int ata_wait_drq() {
    uint32_t timeout = 100000;
    while (timeout--) {
        uint8_t status = inb(ATA_PRIMARY_STATUS);
        if (status & ATA_STATUS_ERR) return -1;
        if (status & ATA_STATUS_DRQ) return 0;
    }
    return -1;
}

/* -------------------------------------------------------
 * ata_init()
 * Detecta el disco ATA y verifica que responde
 * ------------------------------------------------------- */
int ata_init() {
    vga_set_color(VGA_LIGHT_CYAN, VGA_BLACK);
    vga_print("[ATA] Inicializando driver ATA...\n");

    /* Seleccionar slave drive */
    outb(ATA_PRIMARY_DRIVE, 0xB0);

    /* Pequeña espera */
    for (int i = 0; i < 1000; i++) inb(ATA_PRIMARY_STATUS);

    /* Verificar si hay disco */
    uint8_t status = inb(ATA_PRIMARY_STATUS);
    if (status == 0xFF) {
        vga_set_color(VGA_LIGHT_RED, VGA_BLACK);
        vga_print("[ATA] ERROR: No se detecta disco ATA\n");
        return -1;
    }

    vga_set_color(VGA_LIGHT_GREEN, VGA_BLACK);
    vga_print("[ATA] Disco ATA detectado\n");
    vga_set_color(VGA_WHITE, VGA_BLACK);
    vga_print("[ATA] Status: 0x");
    vga_print_hex(status);
    vga_print("\n");

    return 0;
}

/* -------------------------------------------------------
 * ata_read_sector()
 * Lee un sector del disco en modo LBA28
 *
 * lba    → número de sector (0 = primer sector)
 * buffer → destino de 512 bytes
 * ------------------------------------------------------- */
int ata_read_sector(uint32_t lba, uint8_t *buffer) {
    /* Esperar que el disco esté listo */
    if (ata_wait_ready() < 0) return -1;

    /* Enviar comando LBA28 */
    outb(ATA_PRIMARY_SECCOUNT, 1);                    /* 1 sector    */
    outb(ATA_PRIMARY_LBA_LO,  (lba)       & 0xFF);   /* LBA bits 0-7  */
    outb(ATA_PRIMARY_LBA_MID, (lba >> 8)  & 0xFF);   /* LBA bits 8-15 */
    outb(ATA_PRIMARY_LBA_HI,  (lba >> 16) & 0xFF);   /* LBA bits 16-23*/
    outb(ATA_PRIMARY_DRIVE, 0xF0 | ((lba >> 24) & 0x0F)); /* Slave + LBA28 */
    outb(ATA_PRIMARY_COMMAND, ATA_CMD_READ);           /* Comando READ  */

    /* Esperar datos listos */
    if (ata_wait_drq() < 0) return -1;

    /* Leer 256 words = 512 bytes */
    uint16_t *buf16 = (uint16_t *) buffer;
    for (int i = 0; i < 256; i++) {
        buf16[i] = inw(ATA_PRIMARY_DATA);
    }

    return 0;
}

/* -------------------------------------------------------
 * ata_write_sector()
 * Escribe un sector en el disco en modo LBA28
 * ------------------------------------------------------- */
int ata_write_sector(uint32_t lba, uint8_t *buffer) {
    if (ata_wait_ready() < 0) return -1;

    outb(ATA_PRIMARY_SECCOUNT, 1);
    outb(ATA_PRIMARY_LBA_LO,  (lba)       & 0xFF);
    outb(ATA_PRIMARY_LBA_MID, (lba >> 8)  & 0xFF);
    outb(ATA_PRIMARY_LBA_HI,  (lba >> 16) & 0xFF);
    outb(ATA_PRIMARY_DRIVE, 0xF0 | ((lba >> 24) & 0x0F));
    outb(ATA_PRIMARY_COMMAND, ATA_CMD_WRITE);

    if (ata_wait_drq() < 0) return -1;

    /* Escribir 256 words = 512 bytes */
    uint16_t *buf16 = (uint16_t *) buffer;
    for (int i = 0; i < 256; i++) {
        outw(ATA_PRIMARY_DATA, buf16[i]);
    }

    /* Flush cache */
    outb(ATA_PRIMARY_COMMAND, 0xE7);
    ata_wait_ready();

    return 0;
}

/* -------------------------------------------------------
 * ata_print_info()
 * ------------------------------------------------------- */
void ata_print_info() {
    vga_set_color(VGA_LIGHT_CYAN, VGA_BLACK);
    vga_print("[ATA] Driver ATA listo\n");
    vga_set_color(VGA_WHITE, VGA_BLACK);
    vga_print("[ATA] Modo: LBA28\n");
    vga_print("[ATA] Sector size: 512 bytes\n");
    vga_print("[ATA] Bus: Primary (0x1F0)\n");
}
