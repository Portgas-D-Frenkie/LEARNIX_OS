#ifndef VGA_H
#define VGA_H

#include <stdint.h>

// Colores VGA
#define VGA_BLACK        0
#define VGA_BLUE         1
#define VGA_GREEN        2
#define VGA_CYAN         3
#define VGA_RED          4
#define VGA_MAGENTA      5
#define VGA_BROWN        6
#define VGA_LIGHT_GREY   7
#define VGA_DARK_GREY    8
#define VGA_LIGHT_BLUE   9
#define VGA_LIGHT_GREEN  10
#define VGA_LIGHT_CYAN   11
#define VGA_LIGHT_RED    12
#define VGA_LIGHT_MAGENTA 13
#define VGA_YELLOW       14
#define VGA_WHITE        15

// Dimensiones
#define VGA_COLS 80
#define VGA_ROWS 50
#define VGA_ADDRESS 0xB8000

// Funciones
void vga_init();
void vga_clear();
void vga_set_color(uint8_t fg, uint8_t bg);
void vga_putchar(char c);
void vga_print(const char *str);
void vga_print_at(const char *str, int x, int y);
void vga_print_int(int n);
void vga_print_hex(uint32_t n);
#endif
