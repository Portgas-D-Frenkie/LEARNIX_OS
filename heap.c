#include "heap.h"
#include "vga.h"

/* Puntero al inicio del heap */
static block_header_t *heap_start_ptr = 0;

/* -------------------------------------------------------
 * heap_init()
 * Inicializa el heap creando un único bloque libre
 * que ocupa todo el espacio disponible.
 * ------------------------------------------------------- */
void heap_init() {
    heap_start_ptr = (block_header_t *) HEAP_START;

    /* Crear bloque inicial que ocupa todo el heap */
    heap_start_ptr->size = HEAP_SIZE - sizeof(block_header_t);
    heap_start_ptr->used = 0;
    heap_start_ptr->next = 0;
    heap_start_ptr->prev = 0;
}

/* -------------------------------------------------------
 * kmalloc()
 * Busca el primer bloque libre suficientemente grande
 * lo divide si sobra espacio y lo marca como ocupado.
 * ------------------------------------------------------- */
void* kmalloc(uint32_t size) {
    if (size == 0) return 0;

    /* Alinear a 4 bytes */
    if (size % 4 != 0)
        size = size + (4 - size % 4);

    block_header_t *current = heap_start_ptr;

    while (current) {
        /* Buscar bloque libre suficientemente grande */
        if (!current->used && current->size >= size) {

            /* Dividir el bloque si sobra suficiente espacio */
            if (current->size >= size + sizeof(block_header_t) + 4) {
                block_header_t *new_block = (block_header_t *)
                    ((uint8_t *)current + sizeof(block_header_t) + size);

                new_block->size = current->size - size - sizeof(block_header_t);
                new_block->used = 0;
                new_block->next = current->next;
                new_block->prev = current;

                if (current->next)
                    current->next->prev = new_block;

                current->next = new_block;
                current->size = size;
            }

            current->used = 1;
            return (void *)((uint8_t *)current + sizeof(block_header_t));
        }
        current = current->next;
    }

    return 0;  /* Sin memoria disponible */
}

/* -------------------------------------------------------
 * kfree()
 * Libera un bloque marcándolo como libre y fusiona
 * bloques adyacentes libres para evitar fragmentación.
 * ------------------------------------------------------- */
void kfree(void *ptr) {
    if (!ptr) return;

    block_header_t *block = (block_header_t *)
        ((uint8_t *)ptr - sizeof(block_header_t));

    block->used = 0;

    /* Fusionar con bloque siguiente si está libre */
    if (block->next && !block->next->used) {
        block->size += sizeof(block_header_t) + block->next->size;
        block->next  = block->next->next;
        if (block->next)
            block->next->prev = block;
    }

    /* Fusionar con bloque anterior si está libre */
    if (block->prev && !block->prev->used) {
        block->prev->size += sizeof(block_header_t) + block->size;
        block->prev->next  = block->next;
        if (block->next)
            block->next->prev = block->prev;
    }
}

/* -------------------------------------------------------
 * heap_print_info()
 * Muestra el estado del heap en pantalla
 * ------------------------------------------------------- */
void heap_print_info() {
    vga_set_color(VGA_LIGHT_CYAN, VGA_BLACK);
    vga_print("[HEAP] Inicializando kmalloc...\n");

    uint32_t total  = 0;
    uint32_t used   = 0;
    uint32_t free   = 0;
    uint32_t blocks = 0;

    block_header_t *current = heap_start_ptr;
    while (current) {
        blocks++;
        total += current->size;
        if (current->used)
            used += current->size;
        else
            free += current->size;
        current = current->next;
    }

    vga_set_color(VGA_WHITE, VGA_BLACK);
    vga_print("[HEAP] Total:  ");
    vga_print_int(total / 1024);
    vga_print(" KB\n");

    vga_print("[HEAP] Usado:  ");
    vga_print_int(used);
    vga_print(" bytes\n");

    vga_print("[HEAP] Libre:  ");
    vga_print_int(free / 1024);
    vga_print(" KB\n");

    /* Prueba kmalloc */
    vga_set_color(VGA_YELLOW, VGA_BLACK);
    vga_print("[HEAP] Prueba kmalloc:\n");

    void *p1 = kmalloc(64);
    void *p2 = kmalloc(128);
    void *p3 = kmalloc(256);

    vga_set_color(VGA_WHITE, VGA_BLACK);
    vga_print("[HEAP]   kmalloc(64)  = 0x");
    vga_print_hex((uint32_t)p1);
    vga_print("\n");

    vga_print("[HEAP]   kmalloc(128) = 0x");
    vga_print_hex((uint32_t)p2);
    vga_print("\n");

    vga_print("[HEAP]   kmalloc(256) = 0x");
    vga_print_hex((uint32_t)p3);
    vga_print("\n");

    /* Liberar */
    kfree(p1);
    kfree(p2);
    kfree(p3);

    vga_set_color(VGA_LIGHT_GREEN, VGA_BLACK);
    vga_print("[HEAP] kfree OK\n");
    vga_print("[HEAP] Heap listo!\n");
}
