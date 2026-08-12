#ifndef SYSCALL_H
#define SYSCALL_H

#include <stdint.h>

/* ── Numeros de syscall ──────────────────────────── */
#define SYS_EXIT    1   /* Terminar proceso            */
#define SYS_WRITE   2   /* Escribir en pantalla        */
#define SYS_READ    3   /* Leer del teclado            */
#define SYS_OPEN    4   /* Abrir archivo FAT16         */
#define SYS_CLOSE   5   /* Cerrar archivo              */
#define SYS_FREAD   6   /* Leer contenido de archivo   */
#define SYS_EXEC    7   /* Cargar y ejecutar ELF       */
#define SYS_LIST    8   /* Listar directorio raiz      */

/* ── Manejador principal de syscalls ─────────────── */
uint32_t syscall_handler(uint32_t eax, uint32_t ebx,
                         uint32_t ecx, uint32_t edx);

/* ── Funcion auxiliar para invocar syscalls ──────── */
static inline uint32_t syscall(uint32_t num, uint32_t a1,
                               uint32_t a2, uint32_t a3) {
    uint32_t ret;
    __asm__ volatile (
        "int $0x80"
        : "=a"(ret)
        : "a"(num), "b"(a1), "c"(a2), "d"(a3)
        : "memory"
    );
    return ret;
}

#endif
