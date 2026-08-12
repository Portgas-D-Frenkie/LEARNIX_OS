#include "vga.h"

// Puntero a memoria de video
static unsigned short *vga = (unsigned short *) VGA_ADDRESS;

// Estado actual
static int cursor_x = 0;
static int cursor_y = 0;
static uint8_t current_color = 0;

// Crear una entrada VGA (carácter + color)
static inline unsigned short vga_entry(char c, uint8_t color) {
    return (unsigned short) c | ((unsigned short) color << 8);
}

// Crear color combinando fondo y frente
static inline uint8_t vga_color(uint8_t fg, uint8_t bg) {
    return fg | (bg << 4);
}

void vga_init() {
    current_color = vga_color(VGA_LIGHT_GREEN, VGA_BLACK);
    vga_clear();
}

void vga_clear() {
    for (int i = 0; i < VGA_COLS * VGA_ROWS; i++) {
        vga[i] = vga_entry(' ', current_color);
    }
    cursor_x = 0;
    cursor_y = 0;
}

void vga_set_color(uint8_t fg, uint8_t bg) {
    current_color = vga_color(fg, bg);
}

// Scroll: mover todo una línea hacia arriba
static void vga_scroll() {
    for (int y = 0; y < VGA_ROWS - 1; y++) {
        for (int x = 0; x < VGA_COLS; x++) {
            vga[y * VGA_COLS + x] = vga[(y + 1) * VGA_COLS + x];
        }
    }
    // Limpiar última fila
    for (int x = 0; x < VGA_COLS; x++) {
        vga[(VGA_ROWS - 1) * VGA_COLS + x] = vga_entry(' ', current_color);
    }
    cursor_y = VGA_ROWS - 1;
}

void vga_putchar(char c) {
    if (c == '\n') {
        cursor_x = 0;
        cursor_y++;
    } else if (c == '\r') {
        cursor_x = 0;
    } else if (c == '\t') {
        cursor_x = (cursor_x + 4) & ~3;
    } else {
        vga[cursor_y * VGA_COLS + cursor_x] = vga_entry(c, current_color);
        cursor_x++;
    }

    if (cursor_x >= VGA_COLS) {
        cursor_x = 0;
        cursor_y++;
    }

    if (cursor_y >= VGA_ROWS) {
        vga_scroll();
    }
}

void vga_print(const char *str) {
    for (int i = 0; str[i] != '\0'; i++) {
        vga_putchar(str[i]);
    }
}

void vga_print_at(const char *str, int x, int y) {
    cursor_x = x;
    cursor_y = y;
    vga_print(str);
}

// Imprimir número entero
void vga_print_int(int n) {
    if (n < 0) {
        vga_putchar('-');
        n = -n;
    }
    if (n == 0) {
        vga_putchar('0');
        return;
    }
    char buf[12];
    int i = 0;
    while (n > 0) {
        buf[i++] = '0' + (n % 10);
        n /= 10;
    }
    // Imprimir al revés
    for (int j = i - 1; j >= 0; j--) {
        vga_putchar(buf[j]);
    }
}
/* Imprimir número en hexadecimal */
void vga_print_hex(uint32_t n) {
    char hex_chars[] = "0123456789ABCDEF";
    char buf[8];
    for (int i = 7; i >= 0; i--) {
        buf[i] = hex_chars[n & 0xF];
        n >>= 4;
    }
    for (int i = 0; i < 8; i++) {
        vga_putchar(buf[i]);
    }
}
