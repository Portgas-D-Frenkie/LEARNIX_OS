#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdint.h>

// Puerto del teclado PS/2
#define KEYBOARD_DATA_PORT 0x60
char esperar_tecla_char(); // Polling para el puerto 0x60
// Funciones
void keyboard_init();
void keyboard_handler();
void esperar_tecla();

#endif
