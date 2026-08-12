#ifndef CONTEXT_SWITCH_H
#define CONTEXT_SWITCH_H

#include <stdint.h>

/* Realiza el cambio de contexto entre dos procesos
 * old_esp → donde guardar el ESP del proceso actual
 * new_esp → ESP del siguiente proceso a ejecutar    */
void context_switch(uint32_t *old_esp, uint32_t new_esp);

#endif
