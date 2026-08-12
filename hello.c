/* hello.c — Programa de prueba para Learnix OS
 * Se compila como ELF estatico y se carga con elf_load()
 * IMPORTANTE: No puede usar libc — acceso directo a VGA */

/* Escribir directamente en la memoria VGA */
void print_vga(const char *str, int row, int col,
               unsigned char color) {
    unsigned short *vga = (unsigned short *) 0xB8000;
    int pos = row * 80 + col;
    while (*str) {
        vga[pos++] = (color << 8) | (unsigned char)*str;
        str++;
    }
}

/* Punto de entrada del programa */
void _start() {
    /* Escribir en la pantalla VGA */
    print_vga("Hola desde un proceso ELF!", 15, 0, 0x0A);
    print_vga("Learnix OS cargo este ejecutable!", 16, 0, 0x0E);

    /* Terminar: loop infinito sin consumir CPU */
    while (1) {
        __asm__ volatile ("hlt");
    }
}
