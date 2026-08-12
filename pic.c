#include <stdint.h>

#define PIC1_COMMAND 0x20
#define PIC1_DATA    0x21
#define PIC2_COMMAND 0xA0
#define PIC2_DATA    0xA1

// Escribir en un puerto de I/O
static inline void outb(uint16_t port, uint8_t value) {
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

// Leer de un puerto de I/O
static inline uint8_t inb(uint16_t port) {
    uint8_t value;
    __asm__ volatile ("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

void pic_init() {
    // Inicialización en cascada
    outb(PIC1_COMMAND, 0x11);   // Iniciar PIC1
    outb(PIC2_COMMAND, 0x11);   // Iniciar PIC2

    // Remapear IRQs para evitar conflicto con excepciones CPU
    outb(PIC1_DATA, 0x20);      // PIC1 → IRQs 0x20-0x27
    outb(PIC2_DATA, 0x28);      // PIC2 → IRQs 0x28-0x2F

    // Configurar cascada
    outb(PIC1_DATA, 0x04);
    outb(PIC2_DATA, 0x02);

    // Modo 8086
    outb(PIC1_DATA, 0x01);
    outb(PIC2_DATA, 0x01);
    // Habilitar solo IRQ0 (timer) e IRQ1 (teclado)
    outb(PIC1_DATA, 0xFC);  // 11111100 → habilita IRQ0 e IRQ1
    outb(PIC2_DATA, 0xFF);  // Todo enmascarado en PIC2
}
