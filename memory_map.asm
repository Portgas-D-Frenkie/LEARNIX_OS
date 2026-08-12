; =====================================================
; MEMORY_MAP.ASM - Detección de RAM con BIOS E820
;
; Debe llamarse en modo real ANTES de activar
; el modo protegido. Llena una tabla en memoria
; con las regiones de RAM disponibles.
; =====================================================

BITS 16

; Dirección donde guardaremos el mapa de memoria
; Usamos 0x500 que es zona segura en modo real
MEMORY_MAP_ADDR equ 0x500
MEMORY_MAP_COUNT equ 0x4F0     ; Guardamos el contador aquí

global detect_memory
detect_memory:
    ; ES:DI apunta a donde guardar las entradas
    mov ax, 0x0000
    mov es, ax
    mov di, MEMORY_MAP_ADDR     ; dirección destino

    ; Inicializar contador de entradas
    mov dword [MEMORY_MAP_COUNT], 0

    ; EBX = 0 para empezar la secuencia E820
    xor ebx, ebx

    ; EDX = firma mágica 'SMAP' requerida por E820
    mov edx, 0x534D4150         ; 'SMAP' en ASCII

.loop:
    ; Configurar llamada E820
    mov eax, 0xE820             ; función E820
    mov ecx, 24                 ; tamaño de cada entrada (24 bytes)
    int 0x15                    ; llamar al BIOS

    ; Si CF=1 → terminamos o hubo error
    jc .done

    ; Verificar firma de respuesta
    cmp eax, 0x534D4150         ; debe devolver 'SMAP'
    jne .done

    ; Incrementar contador
    inc dword [MEMORY_MAP_COUNT]

    ; Avanzar puntero 24 bytes para siguiente entrada
    add di, 24

    ; Si EBX=0 el BIOS dice que terminó
    test ebx, ebx
    jz .done

    jmp .loop

.done:
    ret
