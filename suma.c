/* =======================================================
 * suma.c — Programa de usuario para Learnix OS
 *
 * Lee dos numeros por teclado, los suma y muestra el
 * resultado. Toda la entrada/salida se hace mediante
 * llamadas al sistema (INT 0x80): el programa nunca
 * toca el hardware directamente.
 *
 * Compilar:
 *   gcc -m32 -ffreestanding -fno-stack-protector \
 *       -fno-builtin -nostdlib -nostartfiles \
 *       -static -no-pie -T user.ld -o suma.elf suma.c
 * ======================================================= */

/* ── Numeros de syscall (deben coincidir con syscall.h) ── */
#define SYS_EXIT    1
#define SYS_WRITE   2
#define SYS_READ    3

/* ── Invocacion de una llamada al sistema ───────────────
 * EAX = numero de syscall
 * EBX = argumento 1
 * ECX = argumento 2
 * EDX = argumento 3
 * El resultado vuelve en EAX. */
static inline unsigned int syscall(unsigned int num, unsigned int a1,
                                   unsigned int a2, unsigned int a3) {
    unsigned int ret;
    __asm__ volatile (
        "int $0x80"
        : "=a"(ret)
        : "a"(num), "b"(a1), "c"(a2), "d"(a3)
        : "memory"
    );
    return ret;
}

/* ── Envolturas para que el codigo se lea mejor ───────── */
static void print(const char *s) {
    syscall(SYS_WRITE, (unsigned int) s, 0, 0);
}

static int read_line(char *buf, int max) {
    return (int) syscall(SYS_READ, (unsigned int) buf,
                         (unsigned int) max, 0);
}

static void exit_program() {
    syscall(SYS_EXIT, 0, 0, 0);
}

/* -------------------------------------------------------
 * str_to_int()
 * Convierte una cadena a entero. Ignora los espacios
 * iniciales y admite signo negativo.
 * ------------------------------------------------------- */
static int str_to_int(const char *s) {
    int resultado = 0;
    int negativo  = 0;
    int i         = 0;

    while (s[i] == ' ' || s[i] == '\t') i++;

    if (s[i] == '-') { negativo = 1; i++; }
    else if (s[i] == '+') { i++; }

    while (s[i] >= '0' && s[i] <= '9') {
        resultado = resultado * 10 + (s[i] - '0');
        i++;
    }

    return negativo ? -resultado : resultado;
}

/* -------------------------------------------------------
 * int_to_str()
 * Convierte un entero a cadena de texto
 * ------------------------------------------------------- */
static void int_to_str(int num, char *buf) {
    int i = 0, j;
    int negativo = 0;

    if (num == 0) {
        buf[0] = '0';
        buf[1] = '\0';
        return;
    }

    if (num < 0) { negativo = 1; num = -num; }

    while (num > 0) {
        buf[i++] = '0' + (num % 10);
        num /= 10;
    }

    if (negativo) buf[i++] = '-';
    buf[i] = '\0';

    /* Invertir la cadena */
    for (j = 0; j < i / 2; j++) {
        char tmp     = buf[j];
        buf[j]       = buf[i - 1 - j];
        buf[i - 1 - j] = tmp;
    }
}

/* -------------------------------------------------------
 * _start — punto de entrada
 * ------------------------------------------------------- */
void _start() {
    char entrada[32];
    char salida[32];
    int  a, b, resultado;

    {
        volatile unsigned short *vga = (unsigned short *) 0xB8000;
        vga[0] = 0x0A00 | 'X';      /* esquina superior izquierda */
    }

    
    print("\n");
    print("=== Calculadora de Suma ===\n");
    print("Programa ELF ejecutandose en Learnix OS\n");
    print("Entrada y salida via INT 0x80\n\n");

    /* ── Primer numero ── */
    print("Primer numero : ");
    read_line(entrada, 32);
    a = str_to_int(entrada);

    /* ── Segundo numero ── */
    print("Segundo numero: ");
    read_line(entrada, 32);
    b = str_to_int(entrada);

    /* ── Calcular y mostrar ── */
    resultado = a + b;

    print("\n");
    int_to_str(a, salida);
    print("   ");
    print(salida);
    print("\n +  ");
    int_to_str(b, salida);
    print(salida);
    print("\n ---------\n = ");
    int_to_str(resultado, salida);
    print(salida);
    print("\n\n");

    print("Programa terminado.\n");

    /* Terminar el proceso mediante la syscall SYS_EXIT */
    exit_program();

    /* Por si SYS_EXIT no retorna el control al scheduler */
    while (1) {
        __asm__ volatile ("hlt");
    }
}
