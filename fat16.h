#ifndef FAT16_H
#define FAT16_H

#include <stdint.h>

/* -------------------------------------------------------
 * Estructura del Boot Sector FAT16
 * ------------------------------------------------------- */
typedef struct {
    uint8_t  jump[3];           /* Instrucción de salto       */
    uint8_t  oem[8];            /* Nombre del OEM             */
    uint16_t bytes_per_sector;  /* Bytes por sector (512)     */
    uint8_t  sectors_per_cluster;
    uint16_t reserved_sectors;  /* Sectores reservados        */
    uint8_t  fat_count;         /* Número de FATs (2)         */
    uint16_t root_entries;      /* Entradas en directorio raíz*/
    uint16_t total_sectors;     /* Total de sectores          */
    uint8_t  media_type;
    uint16_t sectors_per_fat;   /* Sectores por FAT           */
    uint16_t sectors_per_track;
    uint16_t heads;
    uint32_t hidden_sectors;
    uint32_t large_sectors;
} __attribute__((packed)) fat16_boot_t;

/* -------------------------------------------------------
 * Entrada de directorio FAT16 (32 bytes)
 * ------------------------------------------------------- */
typedef struct {
    uint8_t  name[8];       /* Nombre (8 chars, con espacios) */
    uint8_t  ext[3];        /* Extensión (3 chars)            */
    uint8_t  attributes;    /* Atributos del archivo          */
    uint8_t  reserved[10];  /* Reservado                      */
    uint16_t time;          /* Hora de modificación           */
    uint16_t date;          /* Fecha de modificación          */
    uint16_t first_cluster; /* Primer cluster del archivo     */
    uint32_t file_size;     /* Tamaño en bytes                */
} __attribute__((packed)) fat16_entry_t;

/* Atributos de archivo */
#define FAT16_ATTR_READONLY  0x01
#define FAT16_ATTR_HIDDEN    0x02
#define FAT16_ATTR_SYSTEM    0x04
#define FAT16_ATTR_VOLUME    0x08
#define FAT16_ATTR_DIRECTORY 0x10
#define FAT16_ATTR_ARCHIVE   0x20

/* Funciones públicas */
int  fat16_init();
void fat16_list_root();
int  fat16_read_file(const char *name, uint8_t *buffer, uint32_t max_size);
void fat16_print_info();

#endif
