// meminfo.c
#include "../include/shell/shell.h"
#include "../include/video/vga.h"
#include "../include/mm/vmm.h"

void print_mem_stat(const char* label, uint64_t value_mb, size_t pages) {
    vga_set_color(VGA_COLOR_CYAN, VGA_COLOR_BLACK);
    vga_puts("  ");
    vga_puts(label);
    vga_puts(": ");
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_putdec(value_mb, 3);
    vga_puts(" MB ");
    vga_set_color(VGA_COLOR_DARK_GREY, VGA_COLOR_BLACK);
    vga_puts("(");
    vga_putdec(pages, 6);
    vga_puts(" pages)");
    vga_puts("\n");
}

void meminfo_command(const char *args) {
    (void)args;
    size_t total_pages = vmm_get_total_pages();
    size_t used_pages = vmm_get_used_pages();
    size_t free_pages = total_pages - used_pages;
    
    uint64_t total_memory_mb = (total_pages * PAGE_SIZE) / (1024 * 1024);
    uint64_t used_memory_mb = (used_pages * PAGE_SIZE) / (1024 * 1024);
    uint64_t free_memory_mb = (free_pages * PAGE_SIZE) / (1024 * 1024);
    uint8_t used_percent = (used_pages * 100) / total_pages;

    // Header
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLUE);
    vga_puts(" MEMORY INFORMATION ");
    vga_set_color(VGA_COLOR_BLACK, VGA_COLOR_LIGHT_GREY);
    vga_puts(" Physical Memory Status ");
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_puts("\n");

    // Memory stats
    print_mem_stat("Total", total_memory_mb, total_pages);
    print_mem_stat("Used ", used_memory_mb, used_pages);
    print_mem_stat("Free ", free_memory_mb, free_pages);

    // Usage bar
    vga_set_color(VGA_COLOR_CYAN, VGA_COLOR_BLACK);
    vga_puts("  Utilization: ");
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_putdec(used_percent, 3);
    vga_puts("% [");
    
    vga_set_color(VGA_COLOR_RED, VGA_COLOR_BLACK);
    for(int i = 0; i < 25; i++) 
        vga_puts(i < (used_percent/4) ? "█" : " ");
    
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_puts("]\n\n");
}
