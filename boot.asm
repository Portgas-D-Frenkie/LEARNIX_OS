BITS 16
ORG 0x7C00

start:
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00

    mov [boot_drive], dl

    mov si, msg_inicio
    call imprimir16

    ; Cargar kernel
    xor ax, ax
    mov es, ax
    mov ah, 0x02
    mov al, 60
    mov ch, 0
    mov cl, 2
    mov dh, 0
    mov dl, [boot_drive]
    mov bx, 0x1000
    int 0x13
    jc error_disco

    mov si, msg_ok
    call imprimir16

    mov si, msg_pm
    call imprimir16

    cli
    lgdt [gdt_descriptor]
    mov eax, cr0
    or eax, 0x1
    mov cr0, eax
    jmp 0x08:pm32

error_disco:
    mov si, msg_error
    call imprimir16
    jmp error_disco

imprimir16:
    lodsb
    or al, al
    jz .fin
    mov ah, 0x0E
    int 0x10
    jmp imprimir16
.fin:
    ret

msg_inicio  db 'Cargando Learnix...', 0x0D, 0x0A, 0
msg_ok      db 'Kernel cargado OK!', 0x0D, 0x0A, 0
msg_pm      db 'Entrando modo protegido...', 0x0D, 0x0A, 0
msg_error   db 'Error leyendo disco!', 0x0D, 0x0A, 0
boot_drive  db 0

%include "gdt.asm"

BITS 32
pm32:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x90000

    mov dword [0xB8000], 0x0A4B0A4F

    mov eax, 0x1000
    call eax
    cli
    hlt

TIMES 510 - ($ - $$) db 0
DW 0xAA55
