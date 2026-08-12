#include "fat16.h"
#include "ata.h"
#include "vga.h"
#include <stdint.h>

static fat16_boot_t boot_sector;
static uint32_t fat_start    = 0;
static uint32_t root_start   = 0;
static uint32_t data_start   = 0;

/* -------------------------------------------------------
 * fat16_init()
 * Lee el boot sector y calcula las posiciones
 * ------------------------------------------------------- */
int fat16_init() {
    vga_set_color(VGA_LIGHT_CYAN, VGA_BLACK);
    vga_print("[FAT16] Inicializando filesystem...\n");

    /* Leer boot sector (sector 0) */
    uint8_t buffer[512];
    if (ata_read_sector(0, buffer) < 0) {
        vga_set_color(VGA_LIGHT_RED, VGA_BLACK);
        vga_print("[FAT16] ERROR: No se pudo leer el boot sector\n");
        return -1;
    }

    /* Copiar boot sector */
    uint8_t *bs = (uint8_t *) &boot_sector;
    for (int i = 0; i < sizeof(fat16_boot_t); i++) {
        bs[i] = buffer[i];
    }

    /* Calcular posiciones */
    fat_start  = boot_sector.reserved_sectors;
    root_start = fat_start +
                 (boot_sector.fat_count * boot_sector.sectors_per_fat);
    uint32_t root_sectors = (boot_sector.root_entries * 32 + 511) / 512;
    data_start = root_start + root_sectors;

    vga_set_color(VGA_LIGHT_GREEN, VGA_BLACK);
    vga_print("[FAT16] Filesystem montado OK\n");
    vga_set_color(VGA_WHITE, VGA_BLACK);
    vga_print("[FAT16] Bytes/sector: ");
    vga_print_int(boot_sector.bytes_per_sector);
    vga_print("\n[FAT16] FAT inicio: sector ");
    vga_print_int(fat_start);
    vga_print("\n[FAT16] Root inicio: sector ");
    vga_print_int(root_start);
    vga_print("\n[FAT16] Data inicio: sector ");
    vga_print_int(data_start);
    vga_print("\n");

    return 0;
}

/* -------------------------------------------------------
 * fat16_list_root()
 * Lista todos los archivos en el directorio raíz
 * ------------------------------------------------------- */
void fat16_list_root() {
    uint8_t buffer[512];
    uint32_t entries_per_sector = 512 / sizeof(fat16_entry_t);
    uint32_t root_sectors = (boot_sector.root_entries * 32 + 511) / 512;

    vga_set_color(VGA_LIGHT_CYAN, VGA_BLACK);
    vga_print("[FAT16] Directorio raiz:\n");

    int count = 0;
    for (uint32_t s = 0; s < root_sectors; s++) {
        if (ata_read_sector(root_start + s, buffer) < 0) break;

        fat16_entry_t *entries = (fat16_entry_t *) buffer;
        for (uint32_t i = 0; i < entries_per_sector; i++) {
            /* 0x00 = fin de directorio */
            if (entries[i].name[0] == 0x00) goto done;
            /* 0xE5 = entrada borrada */
            if (entries[i].name[0] == 0xE5) continue;
            /* Saltar etiqueta de volumen */
            if (entries[i].attributes & FAT16_ATTR_VOLUME) continue;
            /* Saltar directorios */
            if (entries[i].attributes & FAT16_ATTR_DIRECTORY) continue;

            /* Mostrar archivo */
            vga_set_color(VGA_WHITE, VGA_BLACK);
            vga_print("  ");

            /* Nombre (8 chars) */
            for (int c = 0; c < 8; c++) {
                if (entries[i].name[c] == ' ') break;
                char ch[2] = { entries[i].name[c], '\0' };
                vga_print(ch);
            }

            /* Extensión */
            if (entries[i].ext[0] != ' ') {
                vga_print(".");
                for (int c = 0; c < 3; c++) {
                    if (entries[i].ext[c] == ' ') break;
                    char ch[2] = { entries[i].ext[c], '\0' };
                    vga_print(ch);
                }
            }

            vga_print("  size=");
            vga_print_int(entries[i].file_size);
            vga_print(" bytes  cluster=");
            vga_print_int(entries[i].first_cluster);
            vga_print("\n");
            count++;
        }
    }
done:
    vga_set_color(VGA_LIGHT_GREEN, VGA_BLACK);
    vga_print("[FAT16] Total archivos: ");
    vga_print_int(count);
    vga_print("\n");
}

/* -------------------------------------------------------
 * fat16_read_file()
 * Lee el contenido de un archivo por nombre
 * ------------------------------------------------------- */
int fat16_read_file(const char *name, uint8_t *buffer, uint32_t max_size) {
    uint8_t sector_buf[512];
    uint32_t entries_per_sector = 512 / sizeof(fat16_entry_t);
    uint32_t root_sectors = (boot_sector.root_entries * 32 + 511) / 512;

    for (uint32_t s = 0; s < root_sectors; s++) {
        if (ata_read_sector(root_start + s, sector_buf) < 0) return -1;

        fat16_entry_t *entries = (fat16_entry_t *) sector_buf;
        for (uint32_t i = 0; i < entries_per_sector; i++) {
            if (entries[i].name[0] == 0x00) return -1;
            if (entries[i].name[0] == 0xE5) continue;

            /* Construir nombre completo "NOMBRE  EXT" */
            char fat_name[12] = "           ";
            int ni = 0;
            /* Copiar nombre */
            for (int c = 0; c < 8; c++)
                fat_name[c] = entries[i].name[c];
            /* Copiar extensión */
            fat_name[8]  = entries[i].ext[0];
            fat_name[9]  = entries[i].ext[1];
            fat_name[10] = entries[i].ext[2];
            fat_name[11] = '\0';

            /* Comparar con el nombre buscado */
            int match = 1;
            for (int c = 0; c < 11 && match; c++) {
                if (fat_name[c] != name[c]) match = 0;
            }
            if (!match) continue;

            /* Leer contenido */
            uint32_t cluster = entries[i].first_cluster;
            uint32_t size    = entries[i].file_size;
            if (size > max_size) size = max_size;

            uint32_t sector = data_start +
                (cluster - 2) * boot_sector.sectors_per_cluster;

            uint32_t bytes_read = 0;
            while (bytes_read < size) {
                if (ata_read_sector(sector, sector_buf) < 0) break;
                uint32_t to_copy = 512;
                if (bytes_read + to_copy > size)
                    to_copy = size - bytes_read;
                for (uint32_t b = 0; b < to_copy; b++)
                    buffer[bytes_read + b] = sector_buf[b];
                bytes_read += to_copy;
                sector++;
            }
            return (int) bytes_read;
        }
    }
    return -1;
}
/* -------------------------------------------------------
 * fat16_print_info()
 * ------------------------------------------------------- */
void fat16_print_info() {
    vga_set_color(VGA_LIGHT_CYAN, VGA_BLACK);
    vga_print("[FAT16] Info del filesystem:\n");
    vga_set_color(VGA_WHITE, VGA_BLACK);
    vga_print("[FAT16] OEM: ");
    for (int i = 0; i < 8; i++) {
        char ch[2] = { boot_sector.oem[i], '\0' };
        vga_print(ch);
    }
    vga_print("\n[FAT16] Sectores/cluster: ");
    vga_print_int(boot_sector.sectors_per_cluster);
    vga_print("\n[FAT16] Entradas raiz: ");
    vga_print_int(boot_sector.root_entries);
    vga_print("\n");
}
