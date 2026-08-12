#ifndef PMM_H
#define PMM_H

#include <stdint.h>

/* Tamaño de cada frame de memoria física */
#define PMM_FRAME_SIZE    262144         /* 4 KB por frame        */
#define PMM_FRAMES_MAX    8192        /* máximo 128 MB / 4KB   */
#define PMM_BITMAP_SIZE   (PMM_FRAMES_MAX / 32) /* uint32_t por 32 frames */

/* Dirección donde empieza la memoria libre
 * 0x00200000 = 2 MB (después del kernel) */
#define PMM_START_ADDR    0x00200000

/* Macros para manipular el bitmap */
#define PMM_SET_FRAME(addr)   (pmm_bitmap[(addr) / PMM_FRAME_SIZE / 32] |=  (1 << (((addr) / PMM_FRAME_SIZE) % 32)))
#define PMM_CLEAR_FRAME(addr) (pmm_bitmap[(addr) / PMM_FRAME_SIZE / 32] &= ~(1 << (((addr) / PMM_FRAME_SIZE) % 32)))
#define PMM_TEST_FRAME(addr)  (pmm_bitmap[(addr) / PMM_FRAME_SIZE / 32] &   (1 << (((addr) / PMM_FRAME_SIZE) % 32)))

/* Bitmap global de frames */
extern uint32_t pmm_bitmap[PMM_BITMAP_SIZE];

/* Funciones públicas */
void     pmm_init(uint32_t mem_size);
void*    pmm_alloc_frame();
void     pmm_free_frame(void *addr);
uint32_t pmm_get_free_frames();
uint32_t pmm_get_used_frames();
void     pmm_print_info();

#endif
