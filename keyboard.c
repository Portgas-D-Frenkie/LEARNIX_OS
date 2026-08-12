#include "keyboard.h"
#include "vga.h"
#include <stdint.h>

#define BUFFER_SIZE 256
char buffer_comando[BUFFER_SIZE];
int buffer_idx = 0;
volatile int comando_listo = 0;

static inline uint8_t inb(uint16_t port) {
    uint8_t value;
    __asm__ volatile ("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static inline void outb(uint16_t port, uint8_t value) {
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

static const char scancode_map[128] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
  '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0,  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',   0,
 '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/',   0,  '*',   0, ' '
};

void keyboard_init() {
    buffer_idx = 0;
    buffer_comando[0] = '\0';
    comando_listo = 0;
}

void keyboard_handler() {
    uint8_t scancode = inb(KEYBOARD_DATA_PORT);

    // Si la tecla fue soltada, solo avisamos al PIC y salimos
    if (scancode & 0x80) {
        outb(0x20, 0x20);
        return;
    }

    if (scancode < 128) {
        char c = scancode_map[scancode];

        if (c == '\n') {
            buffer_comando[buffer_idx] = '\0';
            vga_print("\n");
            comando_listo = 1;
        } 
        else if (c == '\b') {
            if (buffer_idx > 0) {
                buffer_idx--;
                // Aquí puedes poner tu función de borrar carácter si la tienes
            }
        } 
        else if (c > 0) {
            if (buffer_idx < BUFFER_SIZE - 1) {
                buffer_comando[buffer_idx++] = c;
                char str[2] = {c, '\0'};
                vga_print(str); 
            }
        }
    }

    /* EOI al PIC Maestro */
    outb(0x20, 0x20);
}
/* -------------------------------------------------------
 * esperar_tecla()
 * Bloquea hasta que comando_listo sea 1 (se presiona Enter)
 * o hasta que se presione cualquier tecla, usando sondeo
 * directo del puerto 0x64/0x60 (sin depender del handler).
 * ------------------------------------------------------- */
void esperar_tecla() {
    uint8_t sc = 0;
    /* Esperar a que se presione una tecla (bit 7 = 0 en el scancode) */
    while (1) {
        if (inb(0x64) & 0x01) {
            sc = inb(0x60);
            if (!(sc & 0x80)) break;  /* tecla presionada (no soltada) */
        }
    }
}
/* -------------------------------------------------------
 * esperar_tecla_char()
 * Espera y retorna un caracter del teclado (bloqueante)
 * ------------------------------------------------------- */
char esperar_tecla_char() {
    uint8_t st, sc;

    while (1) {
        st = inb(0x64);
        if (!(st & 0x01)) continue;

        sc = inb(0x60);

        if (sc & 0x80) continue;
        if (sc >= 128)  continue;

        char c = scancode_map[sc];
        if (c == 0) continue;
        if (sc == 28) {
            vga_print("<ENTER>");
            return '\n';
        }

        if (c == '\n') return '\n';
        if (c == '\b') return '\b';

        char str[2] = { c, '\0' };
        vga_print(str);
        return c;
    }
}
