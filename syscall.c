#include "syscall.h"
#include "vga.h"
#include "process.h"
#include "fat16.h"
#include "elf.h"
#include "keyboard.h"
#include <stdint.h>

/* -------------------------------------------------------
 * syscall_handler()
 * Despacha la llamada al sistema segun el numero en EAX
 *
 * eax → numero de syscall
 * ebx → argumento 1
 * ecx → argumento 2
 * edx → argumento 3
 * Retorna el resultado que ira a EAX
 * ------------------------------------------------------- */
uint32_t syscall_handler(uint32_t eax, uint32_t ebx,
                         uint32_t ecx, uint32_t edx) {
    switch (eax) {

        /* ── SYS_EXIT: terminar el proceso actual ──── */
        case SYS_EXIT:
            vga_set_color(VGA_LIGHT_RED, VGA_BLACK);
            vga_print("[SYS] exit(");
            vga_print_int(ebx);
            vga_print(")\n");
            process_exit();
            return 0;

        /* ── SYS_WRITE: escribir cadena en pantalla ── */
        case SYS_WRITE:
            if (ebx == 0) return 0;
            vga_print((char *) ebx);
            return 1;

        /* ── SYS_READ: leer del teclado ────────────── */
        case SYS_READ: {
            if (ebx == 0 || ecx == 0) return 0;
            char *buf = (char *) ebx;
            uint32_t max = ecx;
            uint32_t i = 0;

            while (i < max - 1) {
                char c = esperar_tecla_char();

                if (c == '\n') break;

                if (c == '\b') {
                    if (i > 0) {
                        i--;
                        vga_print("\b \b");  /* borrar en pantalla */
                    }
                    continue;
                }

                buf[i++] = c;
            }
            buf[i] = '\0';
            return i;
        }

        /* ── SYS_OPEN / SYS_FREAD: leer archivo ────── */
        case SYS_OPEN:
        case SYS_FREAD: {
            if (ebx == 0 || ecx == 0) return 0;
            const char *name = (const char *) ebx;
            uint8_t *buf     = (uint8_t *) ecx;
            uint32_t max     = edx ? edx : 512;
            int bytes = fat16_read_file(name, buf, max);
            return (bytes > 0) ? (uint32_t) bytes : 0;
        }

        /* ── SYS_CLOSE: cerrar descriptor ──────────── */
        case SYS_CLOSE:
            /* Learnix no mantiene tabla de descriptores */
            return 0;

        /* ── SYS_EXEC: cargar y ejecutar ELF ───────── */
        case SYS_EXEC:
            if (ebx == 0) return 0;
            if (elf_load((const char *) ebx) == 0) return 1;
            return 0;

        /* ── SYS_LIST: listar directorio raiz ──────── */
        case SYS_LIST:
            fat16_list_root();
            return 1;

        /* ── Syscall desconocida ───────────────────── */
        default:
            vga_set_color(VGA_LIGHT_RED, VGA_BLACK);
            vga_print("[SYS] Syscall desconocida: ");
            vga_print_int(eax);
            vga_print("\n");
            return (uint32_t) -1;
    }
}
