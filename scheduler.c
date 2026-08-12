#include "scheduler.h"
#include "process.h"
#include "vga.h"
#include <stdint.h>

static uint32_t tick_count        = 0;
static uint8_t  scheduler_running = 0;

extern process_t *current_process;

void scheduler_init() {
    tick_count        = 0;
    scheduler_running = 0;
    vga_set_color(VGA_LIGHT_CYAN, VGA_BLACK);
    vga_print("[SCHED] Scheduler con prioridades inicializado\n");
}

/* -------------------------------------------------------
 * scheduler_tick()
 * Llamada desde irq0 en isr.asm.
 * Recibe ESP actual, retorna ESP del proceso a ejecutar.
 * Usa el quantum de cada proceso según su prioridad.
 * ------------------------------------------------------- */
uint32_t scheduler_tick(uint32_t current_esp) {
    if (!scheduler_running || !current_process)
        return current_esp;

    /* Acumular estadísticas */
    current_process->ticks_total++;

    tick_count++;

    /* Usar quantum propio del proceso según prioridad */
    if (tick_count < current_process->ticks_quantum)
        return current_esp;
    tick_count = 0;

    /* Guardar ESP del proceso actual */
    current_process->esp = current_esp;

    /* Buscar siguiente proceso READY */
    process_t *prev = current_process;
    process_t *next = current_process->next;

    /* Saltar procesos muertos o bloqueados */
    int intentos = 0;
    while ((next->state == PROCESS_DEAD ||
            next->state == PROCESS_BLOCKED) && intentos < MAX_PROCESSES) {
        next = next->next;
        intentos++;
    }

    /* Si solo hay un proceso vivo seguir con él */
    if (next == prev || next->state == PROCESS_DEAD)
        return current_esp;

    /* Cambiar estado */
    prev->state     = PROCESS_READY;
    next->state     = PROCESS_RUNNING;
    current_process = next;

    /* Mostrar estadísticas en tiempo real */
    // process_print_stats();

    return next->esp;
}

/* -------------------------------------------------------
 * scheduler_start()
 * ------------------------------------------------------- */
void scheduler_start() {
    if (!current_process) {
        vga_print("[SCHED] ERROR: sin procesos\n");
        return;
    }

    scheduler_running      = 1;
    current_process->state = PROCESS_RUNNING;

    vga_set_color(VGA_LIGHT_GREEN, VGA_BLACK);
    vga_print("[SCHED] Iniciando scheduler con prioridades...\n");

    uint32_t esp = current_process->esp;
    __asm__ volatile (
        "mov %0, %%esp\n"
        "pop %%gs\n"
        "pop %%fs\n"
        "pop %%es\n"
        "pop %%ds\n"
        ".byte 0x61\n"
        "iret\n"
        : : "r"(esp)
    );
}
