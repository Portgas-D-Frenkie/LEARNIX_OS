#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

// Frecuencia del timer en Hz
#define TIMER_FREQ 100

// Ticks desde que inició el sistema
extern uint32_t timer_ticks;

// Funciones
void timer_init(uint32_t freq);
void timer_handler();
uint32_t timer_get_ticks();

#endif
