#ifndef HEAP_H
#define HEAP_H

#include <stdint.h>

/* Inicio y tamaño del heap */
#define HEAP_START  0x200000    /* 2MB — después del kernel */
#define HEAP_SIZE   0x100000    /* 1MB de heap              */

/* Cabecera de cada bloque */
typedef struct block_header {
    uint32_t size;              /* Tamaño del bloque        */
    uint8_t  used;              /* 1=ocupado, 0=libre       */
    struct block_header *next;  /* Siguiente bloque         */
    struct block_header *prev;  /* Bloque anterior          */
} __attribute__((packed)) block_header_t;

/* Funciones públicas */
void  heap_init();
void* kmalloc(uint32_t size);
void  kfree(void *ptr);
void  heap_print_info();

#endif
