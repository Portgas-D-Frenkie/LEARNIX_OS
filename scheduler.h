#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "process.h"
#include <stdint.h>

#define SCHEDULER_QUANTUM  10

void     scheduler_init();
uint32_t scheduler_tick(uint32_t current_esp);
void     scheduler_start();

extern process_t *current_process;

#endif
