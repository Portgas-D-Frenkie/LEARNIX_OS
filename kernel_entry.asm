BITS 32

section .multiboot
align 4
    dd 0x1BADB002
    dd 0x00
    dd -(0x1BADB002 + 0x00)

section .text
global _start
extern kernel_main

_start:
    ; Stack en zona alta fija lejos del kernel
    mov esp, 0x400000

    xor ebp, ebp
    push ebx
    push eax

    call kernel_main
    cli
    hlt

section .note.GNU-stack noalloc noexec nowrite progbits
