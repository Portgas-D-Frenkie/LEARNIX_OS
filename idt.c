#include "idt.h"

extern void irq0();
extern void irq1();

idt_entry_t idt[256];
idt_ptr_t idt_ptr;

void idt_set_gate(uint8_t num, uint32_t base,
                  uint16_t selector, uint8_t flags)
{
    idt[num].base_low  = base & 0xFFFF;
    idt[num].base_high = (base >> 16) & 0xFFFF;
    idt[num].selector  = selector;
    idt[num].zero      = 0;
    idt[num].flags     = flags;
}

void idt_init()
{
    idt_ptr.limit = sizeof(idt_entry_t) * 256 - 1;
    idt_ptr.base  = (uint32_t)&idt;

    /* Limpiar IDT */
    for (int i = 0; i < 256; i++) {
        idt_set_gate(i, 0, 0, 0);
    }

    /* IRQ0 = Timer */
    idt_set_gate(32, (uint32_t)irq0, 0x08, 0x8E);

    /* IRQ1 = Teclado */
    idt_set_gate(33, (uint32_t)irq1, 0x08, 0x8E);

    /* Cargar IDT */
    __asm__ volatile (
        "lidt %0"
        :
        : "m"(idt_ptr)
        : "memory"
    );
}
