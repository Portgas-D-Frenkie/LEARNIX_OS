#include "timer.h"
#include "vga.h"
#include <stdint.h>

uint32_t timer_ticks = 0;

// Escribir en puerto I/O
static inline void outb(uint16_t port, uint8_t value) {
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

void timer_init(uint32_t freq) {
    // El PIT tiene una frecuencia base de 1193180 Hz
    uint32_t divisor = 1193180 / freq;

    // Comando: canal 0, acceso lobyte/hibyte, modo 3 (square wave)
    outb(0x43, 0x36);

    // Enviar divisor en dos partes
    outb(0x40, (uint8_t)(divisor & 0xFF));
    outb(0x40, (uint8_t)((divisor >> 8) & 0xFF));
}

void timer_handler() {
    timer_ticks++;

    /* No imprimir nada — el shell necesita el cursor libre.
     * Si quieres ver los ticks, usa vga_print_at que no
     * mueve el cursor global. */
}

uint32_t timer_get_ticks() {
    return timer_ticks;
}
