#include "shell.h"
#include "syscall.h"
#include "vga.h"
#include "keyboard.h"
#include "fat16.h"
#include "elf.h"
#include "process.h"
#include <stdint.h>

#define CMD_MAX 64

/* -------------------------------------------------------
 * Utilidades de cadenas (no hay libc)
 * ------------------------------------------------------- */
static int str_eq(const char *a, const char *b) {
    while (*a && *b) {
        if (*a != *b) return 0;
        a++; b++;
    }
    return (*a == *b);
}

static int str_starts(const char *s, const char *pre) {
    while (*pre) {
        if (*s != *pre) return 0;
        s++; pre++;
    }
    return 1;
}

/* Convierte "hola.txt" al formato FAT16 "HOLA    TXT"
 * (8 caracteres de nombre + 3 de extension, en mayusculas) */
static void to_fat_name(const char *in, char *out) {
    int i, j;

    for (i = 0; i < 11; i++) out[i] = ' ';
    out[11] = '\0';

    /* Nombre: hasta el punto, maximo 8 caracteres */
    i = 0;
    while (in[i] && in[i] != '.' && i < 8) {
        char c = in[i];
        if (c >= 'a' && c <= 'z') c -= 32;
        out[i] = c;
        i++;
    }

    /* Extension: los 3 caracteres tras el punto */
    while (in[i] && in[i] != '.') i++;
    if (in[i] == '.') {
        i++;
        j = 8;
        while (in[i] && j < 11) {
            char c = in[i];
            if (c >= 'a' && c <= 'z') c -= 32;
            out[j] = c;
            i++; j++;
        }
    }
}

/* -------------------------------------------------------
 * leer_linea()
 * Lee una linea del teclado con eco y soporte de backspace
 * ------------------------------------------------------- */
static void leer_linea(char *buf, int max) {
    int i = 0;

    while (i < max - 1) {
        char c = esperar_tecla_char();

        if (c == '\n') break;

        if (c == '\b') {
            if (i > 0) {
                i--;
                vga_print("\b \b");   /* borrar en pantalla */
            }
            continue;
        }

        buf[i++] = c;
    }

    buf[i] = '\0';
    vga_print("\n");
}

/* -------------------------------------------------------
 * shell_run()
 * ------------------------------------------------------- */
void shell_run() {
    char    cmd[CMD_MAX];
    char    fatname[12];
    uint8_t filebuf[512];

    /* ── Fase 1: dejar correr la demo de procesos ──────
     * esperar_tecla() hace polling del puerto 0x60. El
     * timer sigue interrumpiendo, asi que Task-A/B/C
     * reciben sus turnos con normalidad. */
    esperar_tecla();

    /* ── Fase 2: tomar el control del CPU ──────────────
     * Bloquea las tareas que sigan vivas para que el
     * scheduler las salte y el shell responda sin cortes. */
    process_block_others();

    /* ── Fase 3: prompt interactivo ────────────────── */
    vga_clear();
    vga_set_color(VGA_LIGHT_GREEN, VGA_BLACK);
    vga_print("=========================================\n");
    vga_print("        Learnix OS Shell  v1.0           \n");
    vga_print("=========================================\n");
    vga_set_color(VGA_WHITE, VGA_BLACK);
    vga_print("Comandos: ls  cat <archivo>  run <elf>  ps  sys  help  exit\n\n");

    while (1) {
        vga_set_color(VGA_LIGHT_CYAN, VGA_BLACK);
        vga_print("learnix> ");
        vga_set_color(VGA_WHITE, VGA_BLACK);

        leer_linea(cmd, CMD_MAX);

        if (cmd[0] == '\0') continue;

        /* ── ls: listar archivos del disco ──────────
         * cli/sti porque el driver ATA usa polling y el
         * timer no debe interrumpir la transferencia. */
        if (str_eq(cmd, "ls")) {
            __asm__ volatile ("cli");
            fat16_list_root();
            __asm__ volatile ("sti");
        }

        /* ── ps: listar procesos ────────────────── */
        else if (str_eq(cmd, "ps")) {
            process_print_all();
        }

        /* ── cat <archivo>: mostrar contenido ───── */
        else if (str_starts(cmd, "cat ")) {
            to_fat_name(cmd + 4, fatname);

            __asm__ volatile ("cli");
            int n = fat16_read_file(fatname, filebuf, 512);
            __asm__ volatile ("sti");

            if (n > 0) {
                filebuf[n] = '\0';
                vga_set_color(VGA_YELLOW, VGA_BLACK);
                vga_print((char *) filebuf);
                vga_print("\n");
            } else {
                vga_set_color(VGA_LIGHT_RED, VGA_BLACK);
                vga_print("Archivo no encontrado: ");
                vga_print(cmd + 4);
                vga_print("\n");
            }
        }

        /* ── run <elf>: cargar y ejecutar programa ──
         * Sin cli/sti: elf_load() llama a process_create()
         * y modificar la lista de procesos con las
         * interrupciones apagadas corrompe el scheduler. */
        else if (str_starts(cmd, "run ")) {
            to_fat_name(cmd + 4, fatname);

            if (elf_load(fatname) == 0) {
                vga_set_color(VGA_LIGHT_GREEN, VGA_BLACK);
                vga_print("Programa cargado. Ejecutando...\n");
                vga_set_color(VGA_WHITE, VGA_BLACK);

                /* Ceder el CPU hasta que el programa termine.
                 * Mientras el shell esta en hlt no lee el
                 * teclado, asi que el programa puede usarlo
                 * sin competencia. */
                while (process_count_active() > 1) {
                    __asm__ volatile ("hlt");
                }

                vga_set_color(VGA_LIGHT_CYAN, VGA_BLACK);
                vga_print("\n[El programa termino]\n");
            } else {
                vga_set_color(VGA_LIGHT_RED, VGA_BLACK);
                vga_print("No se pudo cargar el programa\n");
            }
        }

        /* ── sys: probar la llamada al sistema ──────
         * Demuestra INT 0x80 con SYS_WRITE. */
        else if (str_eq(cmd, "sys")) {
            vga_set_color(VGA_LIGHT_CYAN, VGA_BLACK);
            vga_print("Invocando INT 0x80 con SYS_WRITE...\n");
            vga_set_color(VGA_YELLOW, VGA_BLACK);
            syscall(SYS_WRITE,
                    (uint32_t) "  > Mensaje escrito por una syscall\n",
                    0, 0);
            vga_set_color(VGA_LIGHT_GREEN, VGA_BLACK);
            vga_print("Syscall completada\n");
        }

        /* ── help: ayuda ────────────────────────── */
        else if (str_eq(cmd, "help")) {
            vga_set_color(VGA_LIGHT_CYAN, VGA_BLACK);
            vga_print("Comandos disponibles:\n");
            vga_set_color(VGA_WHITE, VGA_BLACK);
            vga_print("  ls             Listar archivos del disco\n");
            vga_print("  cat <archivo>  Mostrar el contenido de un archivo\n");
            vga_print("  run <elf>      Cargar y ejecutar un programa ELF\n");
            vga_print("  ps             Listar los procesos del sistema\n");
            vga_print("  sys            Probar una llamada al sistema\n");
            vga_print("  help           Mostrar esta ayuda\n");
            vga_print("  exit           Terminar el shell\n");
        }

        /* ── exit: terminar el shell ────────────── */
        else if (str_eq(cmd, "exit")) {
            vga_set_color(VGA_YELLOW, VGA_BLACK);
            vga_print("Cerrando shell...\n");
            process_exit();
        }

        /* ── comando desconocido ────────────────── */
        else {
            vga_set_color(VGA_LIGHT_RED, VGA_BLACK);
            vga_print("Comando desconocido: ");
            vga_print(cmd);
            vga_print("\n");
            vga_set_color(VGA_WHITE, VGA_BLACK);
            vga_print("Escribe 'help' para ver los comandos\n");
        }
    }
}
