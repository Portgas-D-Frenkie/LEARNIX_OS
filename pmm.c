#include "pmm.h"
#include "memory.h"
#include "vga.h"

/* -------------------------------------------------------
 * Bitmap global: cada bit representa un frame de 4KB
 * 0 = libre, 1 = ocupado
 * ------------------------------------------------------- */
uint32_t pmm_bitmap[PMM_BITMAP_SIZE];

/* Estadísticas */
static uint32_t total_frames = 0;
static uint32_t used_frames  = 0;
static uint32_t mem_size_bytes = 0;

/* -------------------------------------------------------
 * pmm_init()
 * Inicializa el PMM con el tamaño total de memoria.
 * Marca como ocupados los primeros 2MB (zona del kernel).
 * Marca como libres el resto de la RAM usable.
 * ------------------------------------------------------- */
void pmm_init(uint32_t mem_size) {
    mem_size_bytes = mem_size;
    total_frames   = mem_size / PMM_FRAME_SIZE;

    /* Marcar todo el bitmap como ocupado por defecto */
    for (int i = 0; i < PMM_BITMAP_SIZE; i++) {
        pmm_bitmap[i] = 0xFFFFFFFF;
    }
    used_frames = total_frames;

    /* Marcar como libres los frames desde PMM_START_ADDR
     * hasta el final de la memoria usable */
    uint32_t start_frame = PMM_START_ADDR / PMM_FRAME_SIZE;
    uint32_t end_frame   = total_frames;

    if (end_frame > PMM_FRAMES_MAX)
        end_frame = PMM_FRAMES_MAX;

    for (uint32_t i = start_frame; i < end_frame; i++) {
        pmm_bitmap[i / 32] &= ~(1 << (i % 32));
        used_frames--;
    }
}

/* -------------------------------------------------------
 * pmm_find_free_frame()
 * Busca el primer frame libre en el bitmap.
 * Retorna el índice del frame o -1 si no hay espacio.
 * ------------------------------------------------------- */
static int pmm_find_free_frame() {
    for (uint32_t i = 0; i < PMM_BITMAP_SIZE; i++) {
        /* Si el uint32_t no está lleno hay al menos un bit libre */
        if (pmm_bitmap[i] != 0xFFFFFFFF) {
            for (int bit = 0; bit < 32; bit++) {
                if (!(pmm_bitmap[i] & (1 << bit))) {
                    return i * 32 + bit;
                }
            }
        }
    }
    return -1;  /* Sin memoria disponible */
}

/* -------------------------------------------------------
 * pmm_alloc_frame()
 * Reserva un frame libre y lo marca como ocupado.
 * Retorna la dirección física del frame o NULL si no hay.
 * ------------------------------------------------------- */
void* pmm_alloc_frame() {
    if (used_frames >= total_frames)
        return 0;   /* Sin memoria disponible */

    int frame = pmm_find_free_frame();
    if (frame == -1)
        return 0;

    /* Marcar frame como ocupado en el bitmap */
    pmm_bitmap[frame / 32] |= (1 << (frame % 32));
    used_frames++;

    /* Retornar la dirección física del frame */
    return (void*)(uint32_t)(frame * PMM_FRAME_SIZE);
}

/* -------------------------------------------------------
 * pmm_free_frame()
 * Libera un frame marcándolo como libre en el bitmap.
 * ------------------------------------------------------- */
void pmm_free_frame(void *addr) {
    uint32_t frame = (uint32_t)addr / PMM_FRAME_SIZE;

    /* Marcar frame como libre */
    pmm_bitmap[frame / 32] &= ~(1 << (frame % 32));

    if (used_frames > 0)
        used_frames--;
}

/* -------------------------------------------------------
 * pmm_get_free_frames()
 * Retorna el número de frames libres disponibles.
 * ------------------------------------------------------- */
uint32_t pmm_get_free_frames() {
    return total_frames - used_frames;
}

/* -------------------------------------------------------
 * pmm_get_used_frames()
 * Retorna el número de frames actualmente ocupados.
 * ------------------------------------------------------- */
uint32_t pmm_get_used_frames() {
    return used_frames;
}

/* -------------------------------------------------------
 * pmm_print_info()
 * Muestra en pantalla el estado del PMM.
 * ------------------------------------------------------- */
void pmm_print_info() {
    vga_set_color(VGA_LIGHT_CYAN, VGA_BLACK);
    vga_print("[PMM] Inicializando gestor de memoria fisica...\n");

    vga_set_color(VGA_WHITE, VGA_BLACK);
    vga_print("[PMM] Total frames : ");
    vga_print_int(total_frames);
    vga_print("\n");

    vga_print("[PMM] Frames usados: ");
    vga_print_int(used_frames);
    vga_print("\n");

    vga_set_color(VGA_LIGHT_GREEN, VGA_BLACK);
    vga_print("[PMM] Frames libres: ");
    vga_print_int(pmm_get_free_frames());
    vga_print("\n");

    /* Prueba: allocar y liberar 3 frames */
    vga_set_color(VGA_YELLOW, VGA_BLACK);
    vga_print("[PMM] Prueba alloc:\n");

    void *f1 = pmm_alloc_frame();
    void *f2 = pmm_alloc_frame();
    void *f3 = pmm_alloc_frame();

    vga_set_color(VGA_WHITE, VGA_BLACK);
    vga_print("[PMM]   frame1 = 0x");
    vga_print_hex((uint32_t)f1);
    vga_print("\n");

    vga_print("[PMM]   frame2 = 0x");
    vga_print_hex((uint32_t)f2);
    vga_print("\n");

    vga_print("[PMM]   frame3 = 0x");
    vga_print_hex((uint32_t)f3);
    vga_print("\n");

    /* Liberar los frames */
    pmm_free_frame(f1);
    pmm_free_frame(f2);
    pmm_free_frame(f3);

    vga_set_color(VGA_LIGHT_GREEN, VGA_BLACK);
    vga_print("[PMM] Frames liberados OK\n");
    vga_print("[PMM] PMM listo!\n");
}
