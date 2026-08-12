#include "idt.h"
#include "vga.h"
#include "timer.h"
#include "keyboard.h"
#include "scheduler.h"

extern void isr0();
extern void isr1();
extern void isr2();
extern void isr3();
extern void isr4();
extern void isr5();
extern void isr6();
extern void isr7();
extern void isr8();
extern void isr13();
extern void isr14();
extern void irq0();
extern void irq1();

static const char *exception_names[] = {
    "Division por cero",
    "Debug",
    "Non-maskable Interrupt",
    "Breakpoint",
    "Overflow",
    "Bound Range Exceeded",
    "Opcode Invalido",
    "Device Not Available",
    "Double Fault",
    "Coprocessor Segment Overrun",
    "Invalid TSS",
    "Segment Not Present",
    "Stack Fault",
    "General Protection Fault",
    "Page Fault"
};

typedef struct {
    uint32_t gs, fs, es, ds;
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
    uint32_t int_no, err_code;
    uint32_t eip, cs, eflags, useresp, ss;
} registers_t;

#define PIC1_COMMAND 0x20
#define PIC2_COMMAND 0xA0

static inline void outb(uint16_t port, uint8_t value) {
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

static void pic_eoi(uint32_t int_no) {
    if (int_no >= 40) outb(PIC2_COMMAND, 0x20);
    outb(PIC1_COMMAND, 0x20);
}

void isr_handler(registers_t *regs) {
    vga_set_color(VGA_RED, VGA_BLACK);
    vga_print_at("EXCEPCION: ", 0, 0);
    if (regs->int_no < 15)
        vga_print(exception_names[regs->int_no]);
    while (1) {}
}

void irq_handler(registers_t *regs) {
    if (regs->int_no == 32 || regs->int_no == 0x08) {
        timer_handler();
    } else if (regs->int_no == 33 || regs->int_no == 0x21 || regs->int_no == 0x0e) {
        keyboard_handler();
    }
    pic_eoi(regs->int_no);
}

void isr_init() {
	idt_set_gate(0,  (uint32_t)isr0,  0x10, 0x8E);
	idt_set_gate(1,  (uint32_t)isr1,  0x10, 0x8E);
	idt_set_gate(2,  (uint32_t)isr2,  0x10, 0x8E);
	idt_set_gate(3,  (uint32_t)isr3,  0x10, 0x8E);
	idt_set_gate(4,  (uint32_t)isr4,  0x10, 0x8E);
	idt_set_gate(5,  (uint32_t)isr5,  0x10, 0x8E);
	idt_set_gate(6,  (uint32_t)isr6,  0x10, 0x8E);
	idt_set_gate(7,  (uint32_t)isr7,  0x10, 0x8E);
	idt_set_gate(8,  (uint32_t)isr8,  0x10, 0x8E);
	idt_set_gate(13, (uint32_t)isr13, 0x10, 0x8E);
	idt_set_gate(14, (uint32_t)isr14, 0x10, 0x8E);

	idt_set_gate(32,   (uint32_t)irq0, 0x10, 0x8E); // Timer
	idt_set_gate(0x21, (uint32_t)irq1, 0x10, 0x8E); // Teclado  ← este era el bug real
}
