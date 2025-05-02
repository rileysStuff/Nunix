// cpuinfo.c
#include "../include/shell/shell.h"
#include "../include/video/vga.h"
#include "../include/kernel/arch/x86/cpu.h"
#include <string.h>

void print_cpu_header(const char* text) {
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLUE);
    vga_puts(" ");
    vga_puts(text);
    vga_puts(" ");
    vga_set_color(VGA_COLOR_BLACK, VGA_COLOR_LIGHT_GREY);
}

void print_cpu_row(const char* label, const char* value) {
    vga_set_color(VGA_COLOR_CYAN, VGA_COLOR_BLACK);
    vga_puts("  ");
    vga_puts(label);
    vga_puts(": ");
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_puts(value);
    vga_puts("\n");
}

void print_feature(cpu_info_t* cpu, const char* label, bool feature) {
    vga_set_color(feature ? VGA_COLOR_GREEN : VGA_COLOR_DARK_GREY, VGA_COLOR_BLACK);
    vga_puts(feature ? " ✓ " : " ✗ ");
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_puts(label);
    vga_puts("  ");
}

// Simple number to string conversion
static void itoa(int num, char* str, int base) {
    int i = 0;
    int is_negative = 0;

    if (num == 0) {
        str[i++] = '0';
        str[i] = '\0';
        return;
    }

    if (num < 0 && base == 10) {
        is_negative = 1;
        num = -num;
    }

    while (num != 0) {
        int rem = num % base;
        str[i++] = (rem > 9) ? (rem - 10) + 'a' : rem + '0';
        num = num / base;
    }

    if (is_negative)
        str[i++] = '-';

    str[i] = '\0';

    // Reverse the string
    int start = 0;
    int end = i - 1;
    while (start < end) {
        char tmp = str[start];
        str[start] = str[end];
        str[end] = tmp;
        start++;
        end--;
    }
}

void cpuinfo_command(const char *args) {
    cpu_info_t cpu;
    cpu_identify(&cpu);
    uint8_t original_color = vga_get_color();

    // Main header
    vga_set_color(VGA_COLOR_YELLOW, VGA_COLOR_BLACK);
    vga_puts("╔════════════════════════════════════════════╗\n");
    vga_puts("║ ");
    print_cpu_header("PROCESSOR INFORMATION");
    vga_puts("             ║\n");
    vga_puts("╚════════════════════════════════════════════╝\n");

    // Basic information
    print_cpu_row("Vendor ID    ", cpu.identity.vendor_id);
    print_cpu_row("Brand String ", cpu.identity.brand_string);
    print_cpu_row("Architecture ", cpu.features.lm ? "x86_64" : "i386");
    
    // Format family/model/stepping manually
    char buf[32];
    char num_buf[12];
    
    // Family
    itoa(cpu.identity.family, num_buf, 10);
    strcpy(buf, num_buf);
    strcat(buf, "/");
    
    // Model
    itoa(cpu.identity.model, num_buf, 10);
    strcat(buf, num_buf);
    strcat(buf, "/");
    
    // Stepping
    itoa(cpu.identity.stepping, num_buf, 10);
    strcat(buf, num_buf);
    
    print_cpu_row("Family/Model ", buf);

    // TSC Frequency
    uint32_t mhz = cpu.tsc_frequency / 1000;
    itoa(mhz, num_buf, 10);
    strcpy(buf, num_buf);
    strcat(buf, " MHz");
    print_cpu_row("TSC Frequency", buf);

    // Features
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLUE);
    vga_puts(" INSTRUCTION SETS ");
    vga_set_color(VGA_COLOR_BLACK, VGA_COLOR_LIGHT_GREY);
    vga_puts("\n");

    print_feature(&cpu, "FPU     ", cpu.features.fpu);
    print_feature(&cpu, "SSE4.2  ", cpu.features.sse4_2);
    print_feature(&cpu, "AVX2    ", cpu.features.avx2);
    vga_puts("\n");
    print_feature(&cpu, "AES-NI  ", cpu.features.aes);
    print_feature(&cpu, "AVX512F ", cpu.features.avx512f);
    vga_puts("\n\n");

    // Cache information
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLUE);
    vga_puts(" CACHE HIERARCHY ");
    vga_set_color(VGA_COLOR_BLACK, VGA_COLOR_LIGHT_GREY);
    vga_puts("\n");

    for (uint8_t i = 0; i < cpu.num_caches; i++) {
        const cache_descriptor_t* c = &cpu.caches[i];
        vga_set_color(VGA_COLOR_CYAN, VGA_COLOR_BLACK);
        vga_puts("  L");
        vga_putdec(c->level, 0);
        
        vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
        vga_puts(" ");
        vga_puts(c->type == CACHE_TYPE_DATA ? "Data     " : 
                c->type == CACHE_TYPE_INSTRUCTION ? "Instr    " : "Unified  ");
        
        vga_puts(" ");
        vga_putdec(c->size / 1024, 4);
        vga_puts(" KB");
        vga_puts("\n");
    }

    vga_set_color(original_color & 0x0F, (original_color >> 4) & 0x0F);
}
