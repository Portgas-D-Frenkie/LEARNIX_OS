# Learnix OS

Sistema operativo educativo de 32 bits para arquitectura x86, desarrollado desde cero
en C y ensamblador como parte del curso de Sistemas Operativos.

**Universidad Nacional de San Antonio Abad del Cusco** · Ingeniería Informática · 2026-I

---
## Autores

- MAMANI CONDORI, Franklin Gilberto
- TTITO HUAMÁN, Jack Eliezer

---

## Descarga rápida

Si solo quieres probar el sistema sin compilarlo, descarga `learnix.iso` desde la
[última release](https://github.com/Portgas-D-Frenkie/LEARNIX_OS/releases/latest) y ejecútala:

```bash
qemu-system-x86_64 -cdrom learnix.iso
```

## Clonar el repositorio

```bash
git clone https://github.com/Portgas-D-Frenkie/LEARNIX_OS.git
cd LEARNIX_OS
```

---

## Características

- Arranque vía GRUB con especificación Multiboot
- Modo protegido de 32 bits con GDT e IDT
- Drivers: VGA (texto), PIT (timer 100 Hz), teclado PS/2
- Gestión de memoria: mapa E820, PMM con bitmap, paginación, heap dinámico
- Multitarea con planificador Round Robin y prioridades
- Driver de disco ATA en modo PIO (LBA28)
- Sistema de archivos FAT16 (lectura)
- Cargador de ejecutables ELF32
- Llamadas al sistema vía `INT 0x80`
- Shell interactivo

---

## Requisitos

Sistema base: Ubuntu / Debian.

```bash
sudo apt update
sudo apt install -y build-essential gcc-multilib nasm \
                    grub-pc-bin grub-common xorriso mtools \
                    qemu-system-x86 dosfstools
```

| Herramienta | Uso |
|---|---|
| `gcc` (con `-m32`) | Compilar el kernel y los programas de usuario |
| `nasm` | Ensamblar los archivos `.asm` |
| `ld` | Enlazar el kernel |
| `grub-mkrescue` + `xorriso` | Generar la ISO arrancable |
| `qemu-system-x86_64` | Emular la máquina |
| `mkfs.fat` | Formatear el disco virtual con FAT16 |

---

## Estructura del proyecto

```
Learnix_OS/
├── kernel_entry.asm      Cabecera Multiboot y punto de entrada
├── isr.asm               Manejadores de interrupciones (IRQ0, IRQ1, INT 0x80)
├── context_switch.asm    Cambio de contexto entre procesos
│
├── kernel.c              kernel_main() — inicialización de todo el sistema
├── vga.c / vga.h         Driver de texto VGA (0xB8000)
├── idt.c / idt.h         Tabla de descriptores de interrupción
├── isr.c                 Despachador de interrupciones en C
├── pic.c / pic.h         Controlador de interrupciones 8259
├── timer.c / timer.h     PIT a 100 Hz
├── keyboard.c/.h         Teclado PS/2 (polling)
│
├── memory.c / memory.h   Mapa de memoria E820 vía Multiboot
├── pmm.c / pmm.h         Physical Memory Manager (bitmap de frames 4 KB)
├── paging.c / paging.h   Paginación — identity map de los primeros 4 MB
├── heap.c / heap.h       kmalloc / kfree (first-fit)
│
├── process.c / process.h PCB y creación de procesos
├── scheduler.c/.h        Round Robin con prioridades
│
├── ata.c / ata.h         Driver de disco ATA (PIO, LBA28)
├── fat16.c / fat16.h     Sistema de archivos FAT16
├── elf.c / elf.h         Cargador de ejecutables ELF32
│
├── syscall.c / syscall.h Llamadas al sistema (INT 0x80)
├── shell.c / shell.h     Shell interactivo
│
├── linker.ld             Script de enlazado del kernel (carga en 0x100000)
├── user.ld               Script de enlazado de programas de usuario (0x300000)
├── build.sh              Script de compilación completa
│
├── iso/boot/grub/grub.cfg  Configuración de GRUB
├── disk.img              Disco virtual de 16 MB con FAT16
└── suma.c, hello.c       Programas de usuario de ejemplo
```

---

## Compilación

### Opción rápida — script automático

```bash
chmod +x build.sh
bash build.sh
```

El script compila, enlaza, genera la ISO y lanza QEMU.

### Opción manual — paso a paso

**1. Ensamblar los archivos ASM**

```bash
nasm -f elf32 kernel_entry.asm   -o kernel_entry.o
nasm -f elf32 isr.asm            -o isr_asm.o
nasm -f elf32 context_switch.asm -o context_switch.o
```

**2. Compilar los archivos C**

```bash
for f in kernel vga idt isr pic timer keyboard memory pmm paging heap \
         process scheduler ata fat16 elf syscall shell; do
    gcc -m32 -fno-stack-protector -fno-builtin -c $f.c -o $f.o || break
done
```

**3. Enlazar el kernel**

```bash
ld -m elf_i386 -T linker.ld -o kernel.bin \
   kernel_entry.o kernel.o vga.o idt.o \
   isr_asm.o isr.o pic.o timer.o keyboard.o \
   memory.o pmm.o paging.o heap.o \
   process.o scheduler.o context_switch.o \
   ata.o fat16.o elf.o syscall.o shell.o
```

> El aviso `LOAD segment with RWX permissions` es normal y no afecta el funcionamiento.

**4. Generar la ISO**

```bash
mkdir -p iso/boot/grub
cp kernel.bin iso/boot/kernel.bin
grub-mkrescue -o learnix.iso iso
```

Contenido de `iso/boot/grub/grub.cfg`:

```
set timeout=0
set default=0
menuentry "Learnix OS" {
    multiboot /boot/kernel.bin
    boot
}
```

---

## Crear el disco virtual

Solo es necesario la primera vez.

```bash
# Crear imagen de 16 MB y formatear con FAT16
dd if=/dev/zero of=disk.img bs=1024 count=16384
mkfs.fat -F 16 -n "LEARNIX" disk.img

# Montar y agregar archivos
mkdir -p /tmp/mnt_learnix
sudo mount -o loop disk.img /tmp/mnt_learnix

echo "Hola desde Learnix OS!"              | sudo tee /tmp/mnt_learnix/HOLA.TXT
echo "Archivo de prueba"                   | sudo tee /tmp/mnt_learnix/TEST.TXT
echo "Sistema de archivos FAT16 funcionando!" | sudo tee /tmp/mnt_learnix/README.TXT

sudo umount /tmp/mnt_learnix
```

### Reglas de nombres FAT16

FAT16 usa el formato **8+3**: máximo 8 caracteres de nombre y 3 de extensión,
en mayúsculas y sin espacios.

| Nombre | Válido | Motivo |
|---|---|---|
| `NOTAS.TXT` | Sí | 5 + 3 caracteres |
| `datos.txt` | Sí | Se convierte a mayúsculas automáticamente |
| `MIARCHIVOLARGO.TXT` | No | Más de 8 caracteres de nombre |
| `mi archivo.txt` | No | Contiene espacios |

---

## Ejecución

```bash
qemu-system-x86_64 \
    -cdrom learnix.iso \
    -drive format=raw,file=disk.img,if=ide,index=1,media=disk \
    -boot order=d \
    -vga std
```

| Parámetro | Función |
|---|---|
| `-cdrom learnix.iso` | Monta la ISO como CD-ROM (Master del bus primario) |
| `-drive ... index=1` | Monta `disk.img` como segundo disco (Slave) |
| `-boot order=d` | Arranca desde el CD-ROM, no desde el disco duro |
| `-vga std` | Adaptador VGA estándar |

> **Importante:** el disco debe estar desmontado antes de lanzar QEMU.
> Si aparece `Failed to get "write" lock`, ejecuta `sudo umount /tmp/mnt_learnix`.

---

## Uso del shell

Al arrancar aparece el shell interactivo:

```
Learnix OS Shell v1.0
Comandos: ls, cat <archivo>, run <elf>, ps, help, exit

learnix>
```

| Comando | Descripción | Ejemplo |
|---|---|---|
| `ls` | Lista los archivos del directorio raíz | `ls` |
| `cat <archivo>` | Muestra el contenido de un archivo | `cat hola.txt` |
| `run <elf>` | Carga y ejecuta un programa ELF | `run suma.elf` |
| `ps` | Lista los procesos activos | `ps` |
| `help` | Muestra la ayuda | `help` |
| `exit` | Termina el shell | `exit` |

Los nombres de archivo pueden escribirse en minúsculas — el shell los convierte
al formato FAT16 internamente.

---

## Compilar programas de usuario

Los programas de usuario se compilan como ELF32 **estático** y se enlazan en la
dirección `0x300000`, dentro de los primeros 4 MB que la paginación mapea con
identity mapping.

```bash
gcc -m32 -ffreestanding -fno-stack-protector -fno-builtin \
    -nostdlib -nostartfiles -static -no-pie \
    -T user.ld \
    -o suma.elf suma.c
```

Verificar que el binario es correcto:

```bash
file suma.elf
# suma.elf: ELF 32-bit LSB executable, Intel i386, statically linked

readelf -l suma.elf | grep LOAD
# Debe mostrar un único segmento LOAD en 0x00300000
```

Si aparecen segmentos en direcciones altas como `0x08048000`, el programa no se
podrá cargar: esa dirección queda fuera del área mapeada por la paginación.
El linker script `user.ld` evita este problema.

Copiar el programa al disco:

```bash
sudo umount /tmp/mnt_learnix 2>/dev/null
sudo mount -o loop disk.img /tmp/mnt_learnix
sudo cp suma.elf /tmp/mnt_learnix/SUMA.ELF
sudo umount /tmp/mnt_learnix
```

Y ejecutarlo desde el shell:

```
learnix> run suma.elf
```

### Restricciones de los programas de usuario

- No pueden usar la biblioteca estándar de C (no hay libc)
- El punto de entrada debe llamarse `_start`
- La salida se escribe directamente en la memoria VGA (`0xB8000`)
- Deben terminar con un bucle `hlt` para no consumir CPU

---

## Detalles de implementación relevantes

### Vectores de interrupción

Con GRUB, el PIC no queda remapeado en los vectores habituales. Los vectores
reales que usa Learnix OS son:

| Vector | Uso | Registro en la IDT |
|---|---|---|
| `0x08` | Timer (IRQ0) | `idt_set_gate(0x08, irq0, 0x10, 0x8E)` |
| `0x0E` | Teclado (IRQ1) | `idt_set_gate(0x0e, irq1, 0x10, 0x8E)` |
| `0x80` | Llamadas al sistema | `idt_set_gate(0x80, int80_handler, 0x10, 0xEE)` |

El selector de segmento es `0x10`, no `0x08`, porque GRUB configura su propia GDT.

El flag `0xEE` del vector `0x80` habilita DPL 3, necesario para que los programas
de usuario puedan invocar `INT 0x80`.

### Máscara del PIC

```c
outb(0x21, 0xFE);   /* Solo IRQ0 habilitada */
```

La IRQ1 (teclado) queda enmascarada a propósito: el teclado se lee por **polling**
del puerto `0x60` desde `esperar_tecla_char()`. Si la IRQ estuviera activa, el
manejador consumiría los scancodes antes de que el shell pudiera leerlos.

### Disco Slave

GRUB ocupa el Master del bus primario con el CD-ROM, por lo que `disk.img` es el
Slave. El driver ATA lo selecciona con:

```c
outb(ATA_PRIMARY_DRIVE, 0xB0);                       /* ata_init         */
outb(ATA_PRIMARY_DRIVE, 0xF0 | ((lba >> 24) & 0x0F)); /* lectura/escritura */
```

### Acceso a disco desde el shell

Las lecturas de disco (`ls`, `cat`) se ejecutan con las interrupciones
deshabilitadas para que el timer no interrumpa el polling del ATA a mitad de una
transferencia:

```c
__asm__ volatile ("cli");
fat16_list_root();
__asm__ volatile ("sti");
```

### Cursor VGA y el timer

`timer_handler()` no debe imprimir en pantalla con funciones que muevan el cursor
global — eso desplaza el punto de escritura del shell a la esquina de la pantalla.
El contador de ticks se mantiene solo en memoria.

---

## Solución de problemas

| Síntoma | Causa | Solución |
|---|---|---|
| `Failed to get "write" lock` | `disk.img` sigue montado | `sudo umount /tmp/mnt_learnix` |
| `This is not a bootable disk` | QEMU arranca desde el disco duro | Añadir `-boot order=d` |
| El teclado no responde | IRQ1 habilitada consumiendo scancodes | `outb(0x21, 0xFE)` |
| Todo se escribe en la esquina | El timer mueve el cursor VGA | Quitar la impresión de `timer_handler()` |
| `ELF no encontrado` | Nombre fuera del formato 8+3 | Verificar con `ls` en el shell |
| El sistema se cuelga al cargar un ELF | Segmentos fuera de los 4 MB mapeados | Compilar con `-T user.ld` |
| `symbol not defined` al enlazar | Falta un `extern` en `isr.asm` | Declarar el símbolo como `extern` |
| `referencia sin definir` a `current_process` | La variable es `static` en `process.c` | Quitar `static` |

---

## Referencias

- Sagi21805. *The LearnixOS Book*. https://www.learnix-os.com
- OSDev Wiki. https://wiki.osdev.org
- Intel. *Intel 64 and IA-32 Architectures Software Developer's Manual*, Vol. 3A.
- Tool Interface Standards. *System V Application Binary Interface* (1995).
- Tanenbaum, A. S. y Bos, H. *Sistemas operativos modernos* (4.ª ed.). Pearson, 2015.
- Silberschatz, A., Galvin, P. B. y Gagne, G. *Fundamentos de sistemas operativos*
  (9.ª ed.). McGraw-Hill, 2018.

---

## Licencia

Distribuido bajo la licencia MIT. Consulta el archivo [LICENSE](LICENSE) para más detalles.

---

## Documentación técnica

Informe completo del proyecto: diseño del bloque de control de procesos, cambio de
contexto, planificador con prioridades, driver ATA, sistema de archivos FAT16 y
cargador de ejecutables ELF.

- [Ver en PDF](SO_2026I_Mamani_Ttito_DocumentacionLearnix/SO_2026I_Mamani_Ttito_DocumentacionLearnix.pdf) — se abre directamente en el navegador
- [Descargar en Word](SO_2026I_Mamani_Ttito_DocumentacionLearnix/SO_2026I_Mamani_Ttito_DocumentacionLearnix.docx) — versión editable
