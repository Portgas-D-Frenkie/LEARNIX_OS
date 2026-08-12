; =====================================================
; CONTEXT_SWITCH.ASM
;
; Realiza el cambio de contexto entre dos procesos.
;
; Prototipo en C:
;   void context_switch(uint32_t *old_esp, uint32_t new_esp);
;
; Argumentos en el stack (cdecl 32-bit):
;   [esp+4] = old_esp (puntero donde guardar el ESP actual)
;   [esp+8] = new_esp (nuevo ESP a cargar)
; =====================================================

BITS 32

global context_switch

context_switch:
    ; ---- Guardar estado del proceso actual ----

    ; Guardar todos los registros de propósito general
    pushad          ; EAX, ECX, EDX, EBX, ESP, EBP, ESI, EDI

    ; Guardar el stack pointer actual en *old_esp
    ; [esp+36] porque pushad empujó 8×4=32 bytes + 4 del ret addr
    mov eax, [esp + 36]     ; eax = old_esp (puntero)
    mov [eax], esp          ; *old_esp = esp actual

    ; ---- Cargar estado del siguiente proceso ----

    ; Cargar el nuevo stack pointer
    mov esp, [esp + 40]     ; esp = new_esp
                            ; (40 porque 32 pushad + 4 ret + 4 old_esp)

    ; Restaurar registros del siguiente proceso
    popad           ; EDI, ESI, EBP, ESP, EBX, EDX, ECX, EAX

    ; Retornar al siguiente proceso
    ; El stack del nuevo proceso tiene: EIP, CS, EFLAGS
    iret

section .note.GNU-stack noalloc noexec nowrite progbits
