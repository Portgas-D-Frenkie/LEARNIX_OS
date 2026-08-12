#include "process.h"
#include "heap.h"
#include "vga.h"
#include <stdint.h>

process_t  *current_process = 0;
static uint32_t next_pid    = 1;

void process_init() {
    current_process = 0;
    next_pid        = 1;
    vga_set_color(VGA_LIGHT_CYAN, VGA_BLACK);
    vga_print("[PROC] Gestor de procesos inicializado\n");
}

process_t* process_create(void (*entry)(), const char *name, uint32_t priority) {
    /* 1. Reservar PCB y stack */
    process_t *proc = (process_t *) kmalloc(sizeof(process_t));
    if (!proc) return 0;

    proc->stack = (uint32_t *) kmalloc(STACK_SIZE);
    if (!proc->stack) { kfree(proc); return 0; }

    /* 2. Configurar PCB */
    proc->pid          = next_pid++;
    proc->state        = PROCESS_READY;
    proc->priority     = priority;
    proc->ticks_total  = 0;
    proc->ticks_quantum = priority * 5;  /* HIGH=20, NORMAL=10, LOW=5 */

    int i = 0;
    while (name[i] && i < 31) { proc->name[i] = name[i]; i++; }
    proc->name[i] = '\0';

    /* 3. Preparar stack inicial */
    uint32_t *p = proc->stack + (STACK_SIZE / sizeof(uint32_t));

    *(--p) = 0x00000202;        /* EFLAGS */
    *(--p) = 0x10;              /* CS     */
    *(--p) = (uint32_t) entry;  /* EIP    */

    *(--p) = 0;  /* EAX */
    *(--p) = 0;  /* ECX */
    *(--p) = 0;  /* EDX */
    *(--p) = 0;  /* EBX */
    *(--p) = 0;  /* ESP dummy */
    *(--p) = 0;  /* EBP */
    *(--p) = 0;  /* ESI */
    *(--p) = 0;  /* EDI */

    *(--p) = 0x10;  /* DS */
    *(--p) = 0x10;  /* ES */
    *(--p) = 0x10;  /* FS */
    *(--p) = 0x10;  /* GS */

    proc->esp = (uint32_t) p;

    /* 4. Agregar a lista circular */
    if (!current_process) {
        proc->next      = proc;
        current_process = proc;
    } else {
        process_t *tmp = current_process;
        while (tmp->next != current_process) tmp = tmp->next;
        tmp->next  = proc;
        proc->next = current_process;
    }

    vga_set_color(VGA_WHITE, VGA_BLACK);
    vga_print("[PROC] Creado PID=");
    vga_print_int(proc->pid);
    vga_print(" [");
    vga_print(proc->name);
    vga_print("] prioridad=");
    vga_print_int(proc->priority);
    vga_print(" quantum=");
    vga_print_int(proc->ticks_quantum);
    vga_print("\n");

    return proc;
}

/* -------------------------------------------------------
 * process_exit()
 * Marca el proceso actual como muerto
 * ------------------------------------------------------- */
void process_exit() {
    if (!current_process) return;
    current_process->state = PROCESS_DEAD;
    vga_set_color(VGA_LIGHT_RED, VGA_BLACK);
    vga_print("[PROC] Proceso PID=");
    vga_print_int(current_process->pid);
    vga_print(" [");
    vga_print(current_process->name);
    vga_print("] terminado\n");
    /* El scheduler lo saltará en el próximo tick */
    while (1) { __asm__ volatile ("hlt"); }
}

/* -------------------------------------------------------
 * process_block() / process_unblock()
 * ------------------------------------------------------- */
void process_block(process_t *proc) {
    if (proc) proc->state = PROCESS_BLOCKED;
}

void process_unblock(process_t *proc) {
    if (proc) proc->state = PROCESS_READY;
}

process_t* process_get_current() {
    return current_process;
}

/* -------------------------------------------------------
 * process_print_all()
 * ------------------------------------------------------- */
void process_print_all() {
    if (!current_process) {
        vga_print("[PROC] Sin procesos\n");
        return;
    }
    vga_set_color(VGA_LIGHT_CYAN, VGA_BLACK);
    vga_print("[PROC] Lista de procesos:\n");

    process_t *p = current_process;
    do {
        vga_set_color(VGA_WHITE, VGA_BLACK);
        vga_print("  PID=");
        vga_print_int(p->pid);
        vga_print(" [");
        vga_print(p->name);
        vga_print("] pri=");
        vga_print_int(p->priority);
        vga_print(" q=");
        vga_print_int(p->ticks_quantum);
        vga_print(" estado=");
        if (p->state == PROCESS_READY)   { vga_set_color(VGA_LIGHT_GREEN, VGA_BLACK); vga_print("READY");   }
        if (p->state == PROCESS_RUNNING) { vga_set_color(VGA_YELLOW,      VGA_BLACK); vga_print("RUNNING"); }
        if (p->state == PROCESS_BLOCKED) { vga_set_color(VGA_LIGHT_RED,   VGA_BLACK); vga_print("BLOCKED"); }
        if (p->state == PROCESS_DEAD)    { vga_set_color(VGA_RED,         VGA_BLACK); vga_print("DEAD");    }
        vga_print("\n");
        p = p->next;
    } while (p != current_process);
}

/* -------------------------------------------------------
 * process_print_stats()
 * Muestra estadísticas de uso de CPU por proceso
 * ------------------------------------------------------- */
void process_print_stats() {
    if (!current_process) return;

    vga_set_color(VGA_LIGHT_CYAN, VGA_BLACK);
    vga_print_at("[PROC] Estadisticas CPU:", 0, 22);

    process_t *p = current_process;
    int row = 23;
    do {
        vga_set_color(VGA_WHITE, VGA_BLACK);
        /* Nombre del proceso */
        vga_print_at(p->name, 0, row);
        vga_print_at(" ticks=", 8, row);

        /* Número de ticks */
        char buf[10];
        uint32_t n = p->ticks_total;
        int idx = 9;
        buf[idx] = '\0';
        if (n == 0) { buf[--idx] = '0'; }
        else { while (n > 0) { buf[--idx] = '0' + (n % 10); n /= 10; } }
        vga_print_at(buf + idx, 15, row);

        row++;
        p = p->next;
    } while (p != current_process && row < 25);
}
/* -------------------------------------------------------
 * process_count_alive()
 * Cuenta los procesos que no estan en estado DEAD
 * ------------------------------------------------------- */
int process_count_alive() {
    if (!current_process) return 0;

    int n = 0;
    process_t *p = current_process;
    do {
        if (p->state != PROCESS_DEAD) n++;
        p = p->next;
    } while (p != current_process);

    return n;
}
/* -------------------------------------------------------
 * process_block_others()
 * Marca como BLOCKED todos los procesos vivos excepto el
 * actual. El scheduler los saltara, dejando el CPU
 * disponible para el proceso que llama a esta funcion.
 * ------------------------------------------------------- */
void process_block_others() {
    if (!current_process) return;

    process_t *p = current_process->next;
    while (p != current_process) {
        if (p->state != PROCESS_DEAD)
            p->state = PROCESS_BLOCKED;
        p = p->next;
    }
}
/* -------------------------------------------------------
 * process_count_active()
 * Cuenta los procesos en estado READY o RUNNING.
 * Los procesos BLOCKED y DEAD no se cuentan: el scheduler
 * los salta y no compiten por el CPU.
 * ------------------------------------------------------- */
int process_count_active() {
    if (!current_process) return 0;

    int n = 0;
    process_t *p = current_process;
    do {
        if (p->state == PROCESS_READY || p->state == PROCESS_RUNNING)
            n++;
        p = p->next;
    } while (p != current_process);

    return n;
}
