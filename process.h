#ifndef PROCESS_H
#define PROCESS_H

#include <stdint.h>

/* Estados */
#define PROCESS_READY    0
#define PROCESS_RUNNING  1
#define PROCESS_BLOCKED  2
#define PROCESS_DEAD     3

/* Prioridades */
#define PRIORITY_LOW     1
#define PRIORITY_NORMAL  2
#define PRIORITY_HIGH    4

/* Tamaño del stack por proceso (16 KB) */
#define STACK_SIZE       16384
#define MAX_PROCESSES    16

/* -------------------------------------------------------
 * PCB — Process Control Block
 * ------------------------------------------------------- */
typedef struct process {
    uint32_t pid;           /* ID único del proceso          */
    uint32_t state;         /* Estado actual                 */
    uint32_t priority;      /* Prioridad: 1=LOW 2=NORMAL 4=HIGH */
    uint32_t esp;           /* Stack pointer guardado        */
    uint32_t *stack;        /* Base del stack                */
    char     name[32];      /* Nombre del proceso            */
    uint32_t ticks_total;   /* Total de ticks ejecutados     */
    uint32_t ticks_quantum; /* Quantum según prioridad       */
    struct process *next;   /* Lista circular                */
} process_t;

/* Funciones públicas */
int process_count_alive();
void process_block_others();
void       process_init();
process_t* process_create(void (*entry)(), const char *name, uint32_t priority);
void       process_exit();
void       process_block(process_t *proc);
void       process_unblock(process_t *proc);
process_t* process_get_current();
void       process_print_all();
void       process_print_stats();
int process_count_active();

#endif
