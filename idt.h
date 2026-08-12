#ifndef IDT_H
#define IDT_H

#include <stdint.h>

// Estructura de una entrada en la IDT (8 bytes)
typedef struct {
    uint16_t base_low;      // Bits 0-15 de la dirección del manejador
    uint16_t selector;      // Selector de segmento de código (0x08)
    uint8_t  zero;          // Siempre 0
    uint8_t  flags;         // Tipo y atributos
    uint16_t base_high;     // Bits 16-31 de la dirección del manejador
} __attribute__((packed)) idt_entry_t;

// Estructura del descriptor de la IDT (para LIDT)
typedef struct {
    uint16_t limit;         // Tamaño de la IDT menos 1
    uint32_t base;          // Dirección de la IDT
} __attribute__((packed)) idt_ptr_t;

// Función para inicializar la IDT
void idt_init();

// Función para registrar una entrada en la IDT
void idt_set_gate(uint8_t num, uint32_t base, uint16_t selector, uint8_t flags);

#endif
