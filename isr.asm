BITS 32
extern syscall_handler
extern isr_handler
extern irq_handler
extern keyboard_handler
extern timer_handler
extern scheduler_tick

global isr0, isr1, isr2, isr3, isr4, isr5, isr6, isr7
global isr8, isr13, isr14
global irq0, irq1
global idt_load
global int80_handler
idt_load:
    mov eax, [esp+4]
    lidt [eax]
    ret

; Excepciones CPU
isr0:  iret
isr1:  iret
isr2:  iret
isr3:  iret
isr4:  iret
isr5:  iret
isr6:  iret
isr7:  iret
isr8:  add esp, 4
       iret
isr13: add esp, 4
       iret
isr14: add esp, 4
       iret

; IRQ0 — Timer con context switch

irq0:
    pushad
    push ds
    push es
    push fs
    push gs

    mov ax, 0x10
    mov ds, ax
    mov es, ax

    call timer_handler

    ; EOI antes del switch
    mov al, 0x20
    out 0x20, al

    ; Pasar ESP actual al scheduler
    ; Recibir nuevo ESP en EAX
    push esp
    call scheduler_tick
    add esp, 4
    mov esp, eax      ; cargar nuevo ESP (puede ser el mismo si no hay switch)

    pop gs
    pop fs
    pop es
    pop ds
    popad
    iret

; IRQ1 — Teclado
irq1:
    pushad
    call keyboard_handler
    mov al, 0x20
    out 0x20, al
    popad
    iret

section .note.GNU-stack noalloc noexec nowrite progbits
; ─────────────────────────────────────────────────────
; INT 0x80 — Manejador de llamadas al sistema
; Recibe: EAX=numero, EBX/ECX/EDX=argumentos
; Retorna: EAX=resultado
; ─────────────────────────────────────────────────────
int80_handler:
    ; Guardar segmentos
    push ds
    push es
    push fs
    push gs

    mov ax, 0x10        ; selector de datos del kernel
    mov ds, ax
    mov es, ax

    ; Empujar argumentos en orden inverso (cdecl)
    push edx            ; arg 3
    push ecx            ; arg 2
    push ebx            ; arg 1
    push eax            ; numero de syscall

    call syscall_handler

    add esp, 16         ; limpiar los 4 argumentos
                        ; EAX contiene el resultado

    ; Restaurar segmentos
    pop gs
    pop fs
    pop es
    pop ds

    iret
