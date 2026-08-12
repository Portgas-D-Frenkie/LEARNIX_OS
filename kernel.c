/* =======================================================
 * kernel.c — Learnix OS
 * Punto de entrada del kernel y secuencia de demostración
 *
 * Pantallas:
 *   1. Arranque + gestión de memoria
 *   2. Driver ATA + sistema de archivos FAT16
 *   3. Lectura de archivos
 *   4. Process Management (Task-A/B/C con prioridades)
 *   5. Shell interactivo
 * ======================================================= */

#include "vga.h"
#include "idt.h"
#include "timer.h"
#include "keyboard.h"
#include "memory.h"
#include "pmm.h"
#include "paging.h"
#include "heap.h"
#include "process.h"
#include "scheduler.h"
#include "ata.h"
#include "fat16.h"
#include "elf.h"
#include "syscall.h"
#include "shell.h"
#include <stdint.h>

extern void isr_init();
extern void irq0();
extern void irq1();
extern void int80_handler();

static inline void outb(uint16_t port, uint8_t value) {
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

/* -------------------------------------------------------
 * Tareas de demostración
 *
 * Los delays están escalados de forma INVERSA a la
 * prioridad para que la diferencia de CPU sea visible:
 *   Task-A (HIGH)   delay 8M  → avanza rápido
 *   Task-B (NORMAL) delay 16M → mitad de velocidad
 *   Task-C (LOW)    delay 32M → un cuarto de velocidad
 *
 * Cada tarea termina con process_exit() tras 40 iteraciones
 * para dejar el CPU libre al shell.
 * ------------------------------------------------------- */
#define DEMO_ITERACIONES 40

void task_a() {                     /* HIGH — quantum 20 */
    int col = 0;
    while (1) {
        volatile int i;
        for (i = 0; i < 8000000; i++) {}
        vga_set_color(VGA_LIGHT_GREEN, VGA_BLACK);
        vga_print_at("A", col % 40, 18);
        col++;
    }
}

void task_b() {                     /* NORMAL — quantum 10 */
    int col = 0;
    while (1) {
        volatile int i;
        for (i = 0; i < 16000000; i++) {}
        vga_set_color(VGA_LIGHT_BLUE, VGA_BLACK);
        vga_print_at("B", col % 40, 19);
        col++;
    }
}

void task_c() {                     /* LOW — quantum 5 */
    int col = 0;
    while (1) {
        volatile int i;
        for (i = 0; i < 32000000; i++) {}
        vga_set_color(VGA_YELLOW, VGA_BLACK);
        vga_print_at("C", col % 40, 20);
        col++;
    }
}

/* -------------------------------------------------------
 * kernel_main
 * ------------------------------------------------------- */
void kernel_main(uint32_t magic, multiboot_info_t *mbi) {

    /* ═══════════════════════════════════════════════════
     * PANTALLA 1 — Arranque y gestión de memoria
     * ═══════════════════════════════════════════════════ */
    vga_init();
    vga_set_color(VGA_LIGHT_GREEN, VGA_BLACK);
    vga_print("=========================================\n");
    vga_print("           L E A R N I X   O S           \n");
    vga_print("=========================================\n");
    vga_set_color(VGA_WHITE, VGA_BLACK);
    vga_print("Sistema operativo x86 32-bit\n");
    vga_print("UNSAAC - Sistemas Operativos - 2026-I\n\n");

    /* ---- IDT y PIC ---- */
    idt_init();
    isr_init();

    /* Remapeo del PIC */
    outb(0x20, 0x11); outb(0xA0, 0x11);
    outb(0x21, 0x20); outb(0xA1, 0x28);
    outb(0x21, 0x04); outb(0xA1, 0x02);
    outb(0x21, 0x01); outb(0xA1, 0x01);
    outb(0x21, 0xFF); outb(0xA1, 0xFF);

    /* Vectores reales con GRUB: timer 0x08, teclado 0x0E.
     * Selector 0x10 porque GRUB instala su propia GDT.
     * 0xEE en el vector 0x80 habilita DPL 3 (modo usuario). */
    idt_set_gate(0x08, (uint32_t)irq0,          0x10, 0x8E);
    idt_set_gate(0x0e, (uint32_t)irq1,          0x10, 0x8E);
    idt_set_gate(0x80, (uint32_t)int80_handler, 0x10, 0xEE);

    vga_set_color(VGA_LIGHT_CYAN, VGA_BLACK);
    vga_print("[IDT] Tabla de interrupciones cargada\n");
    vga_set_color(VGA_WHITE, VGA_BLACK);
    vga_print("[IDT] Timer=0x08  Teclado=0x0E  Syscall=0x80\n");

    /* ---- Timer PIT a 100 Hz ---- */
    uint32_t divisor = 1193180 / 100;
    outb(0x43, 0x36);
    outb(0x40, (uint8_t)(divisor & 0xFF));
    outb(0x40, (uint8_t)((divisor >> 8) & 0xFF));

    /* Solo IRQ0 habilitada. La IRQ1 queda enmascarada:
     * el teclado se lee por polling desde el shell. */
    outb(0x21, 0xFE);
    outb(0xA1, 0xFF);

    vga_set_color(VGA_LIGHT_CYAN, VGA_BLACK);
    vga_print("[PIT] Timer configurado a 100 Hz\n\n");

    /* ---- Gestión de memoria ---- */
    memory_init_multiboot(magic, mbi);
    pmm_init(memory_get_total());
    paging_init();
    heap_init();

    vga_set_color(VGA_LIGHT_GREEN, VGA_BLACK);
    vga_print("\nMemory Management OK\n");

    vga_set_color(VGA_YELLOW, VGA_BLACK);
    vga_print("\n[Pulsa una tecla: Driver ATA + FAT16]\n");
    esperar_tecla();
    vga_clear();

    /* ═══════════════════════════════════════════════════
     * PANTALLA 2 — Driver ATA y sistema de archivos
     * ═══════════════════════════════════════════════════ */
    vga_set_color(VGA_LIGHT_GREEN, VGA_BLACK);
    vga_print("=== Driver de Disco y Sistema de Archivos ===\n\n");

    if (ata_init() == 0) {
        ata_print_info();
        vga_print("\n");
        fat16_init();
        vga_print("\n");
        fat16_print_info();
        vga_print("\n");
        fat16_list_root();
    } else {
        vga_set_color(VGA_LIGHT_RED, VGA_BLACK);
        vga_print("ERROR: no se detecto el disco ATA\n");
    }

    vga_set_color(VGA_YELLOW, VGA_BLACK);
    vga_print("\n[Pulsa una tecla: contenido de los archivos]\n");
    esperar_tecla();
    vga_clear();

    /* ═══════════════════════════════════════════════════
     * PANTALLA 3 — Lectura de archivos desde FAT16
     * ═══════════════════════════════════════════════════ */
    vga_set_color(VGA_LIGHT_GREEN, VGA_BLACK);
    vga_print("=== Lectura de Archivos desde FAT16 ===\n\n");

    uint8_t file_buf[512];
    int bytes;

    vga_set_color(VGA_LIGHT_CYAN, VGA_BLACK);
    vga_print("HOLA.TXT\n");
    vga_set_color(VGA_WHITE, VGA_BLACK);
    bytes = fat16_read_file("HOLA    TXT", file_buf, 512);
    if (bytes > 0) {
        file_buf[bytes] = '\0';
        vga_print("  > ");
        vga_print((char *) file_buf);
        vga_print("\n");
    }

    vga_set_color(VGA_LIGHT_CYAN, VGA_BLACK);
    vga_print("\nTEST.TXT\n");
    vga_set_color(VGA_WHITE, VGA_BLACK);
    bytes = fat16_read_file("TEST    TXT", file_buf, 512);
    if (bytes > 0) {
        file_buf[bytes] = '\0';
        vga_print("  > ");
        vga_print((char *) file_buf);
        vga_print("\n");
    }

    vga_set_color(VGA_LIGHT_CYAN, VGA_BLACK);
    vga_print("\nREADME.TXT\n");
    vga_set_color(VGA_WHITE, VGA_BLACK);
    bytes = fat16_read_file("README  TXT", file_buf, 512);
    if (bytes > 0) {
        file_buf[bytes] = '\0';
        vga_print("  > ");
        vga_print((char *) file_buf);
        vga_print("\n");
    }

    vga_set_color(VGA_YELLOW, VGA_BLACK);
    vga_print("\n[Pulsa una tecla: Process Management]\n");
    esperar_tecla();
    vga_clear();

    /* ═══════════════════════════════════════════════════
     * PANTALLA 4 y 5 — Process Management y shell
     *
     * Las tres tareas terminan solas con process_exit().
     * Al acabar, el shell queda con el CPU disponible.
     * ═══════════════════════════════════════════════════ */
    vga_set_color(VGA_LIGHT_GREEN, VGA_BLACK);
    vga_print("=== Process Management ===\n\n");

    process_init();
    scheduler_init();
    vga_print("\n");

    /* IMPORTANTE: process_init() antes de crear procesos.
     * Si elf_load() se llamara antes, la lista circular
     * quedaria corrupta. */
    process_create(task_a,    "Task-A", PRIORITY_HIGH);
    process_create(task_b,    "Task-B", PRIORITY_NORMAL);
    process_create(task_c,    "Task-C", PRIORITY_LOW);
    process_create(shell_run, "Shell",  PRIORITY_HIGH);

    vga_print("\n");
    process_print_all();

    vga_set_color(VGA_WHITE, VGA_BLACK);
    vga_print("\nMultitarea con prioridades:\n");
    vga_print("  A = HIGH   (quantum 20)  fila 18\n");
    vga_print("  B = NORMAL (quantum 10)  fila 19\n");
    vga_print("  C = LOW    (quantum 5)   fila 20\n");
    vga_set_color(VGA_YELLOW, VGA_BLACK);
    vga_print("\n[Pulsa una tecla para abrir el shell]\n\n");

    /* Habilitar interrupciones y arrancar el scheduler */
    __asm__ volatile ("sti");
    scheduler_start();

    /* No se alcanza nunca */
    while (1) {}
}
