#!/bin/bash
set -e

echo "=== 1. Ensamblando archivos ASM ==="
nasm -f elf32 kernel_entry.asm  -o kernel_entry.o
nasm -f elf32 isr.asm           -o isr_asm.o
nasm -f elf32 context_switch.asm -o context_switch.o

echo "=== 2. Compilando archivos C ==="
gcc -m32 -fno-stack-protector -fno-builtin -c kernel.c    -o kernel.o
gcc -m32 -fno-stack-protector -fno-builtin -c vga.c       -o vga.o
gcc -m32 -fno-stack-protector -fno-builtin -c idt.c       -o idt.o
gcc -m32 -fno-stack-protector -fno-builtin -c isr.c       -o isr.o
gcc -m32 -fno-stack-protector -fno-builtin -c pic.c       -o pic.o
gcc -m32 -fno-stack-protector -fno-builtin -c timer.c     -o timer.o
gcc -m32 -fno-stack-protector -fno-builtin -c keyboard.c  -o keyboard.o
gcc -m32 -fno-stack-protector -fno-builtin -c memory.c    -o memory.o
gcc -m32 -fno-stack-protector -fno-builtin -c pmm.c       -o pmm.o
gcc -m32 -fno-stack-protector -fno-builtin -c paging.c    -o paging.o
gcc -m32 -fno-stack-protector -fno-builtin -c heap.c      -o heap.o
gcc -m32 -fno-stack-protector -fno-builtin -c process.c   -o process.o
gcc -m32 -fno-stack-protector -fno-builtin -c scheduler.c -o scheduler.o

echo "=== 3. Enlazando el Kernel ==="
ld -m elf_i386 -T linker.ld -o kernel.bin \
   kernel_entry.o kernel.o vga.o idt.o \
   isr_asm.o isr.o pic.o timer.o keyboard.o \
   memory.o pmm.o paging.o heap.o \
   process.o scheduler.o context_switch.o

echo "=== 4. Creando ISO ==="
mkdir -p iso/boot/grub
cp kernel.bin iso/boot/kernel.bin

if [ ! -f iso/boot/grub/grub.cfg ]; then
cat << EOF > iso/boot/grub/grub.cfg
set timeout=0
set default=0
menuentry "Learnix OS" {
    multiboot /boot/kernel.bin
    boot
}
EOF
fi

echo "=== 5. Generando learnix.iso ==="
grub-mkrescue -o learnix.iso iso

echo "=== 6. Lanzando QEMU ==="
qemu-system-x86_64 -cdrom learnix.iso -vga std
