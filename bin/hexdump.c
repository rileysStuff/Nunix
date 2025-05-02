#include <stdint.h>
#include <stddef.h>
#include "../include/mm/vmm.h"
#include "../include/video/vga.h"

static void print_byte(uint8_t byte) {
    const char hex_chars[] = "0123456789ABCDEF";
    vga_putchar(hex_chars[(byte >> 4) & 0xF]);
    vga_putchar(hex_chars[byte & 0xF]);
}

static void print_address(uint32_t addr) {
    vga_puthex(addr);
    vga_puts(": ");
}

static void print_ascii(uint8_t* data, size_t count) {
    vga_puts(" | ");
    for (size_t i = 0; i < count; i++) {
        uint8_t c = data[i];
        vga_putchar((c >= 32 && c <= 126) ? c : '.');
    }
    vga_puts(" |\n");
}

void hexdump(uint32_t* virtual_addr, size_t bytes_to_dump) {
    if (!virtual_addr || bytes_to_dump == 0) {
        vga_puts("Invalid parameters\n");
        return;
    }

    uint8_t original_color = vga_get_color();
    vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);

    uint8_t* current = (uint8_t*)virtual_addr;
    uint8_t* end = current + bytes_to_dump;

    while (current < end) {
        print_address((uint32_t)current);
        
        // Print 16 bytes per line
        for (int i = 0; i < 16; i++) {
            if (current + i < end) {
                print_byte(current[i]);
            } else {
                vga_puts("  "); // Padding
            }
            
            // Add space between bytes
            vga_putchar(' ');
            
            // Extra space every 4 bytes
            if (i % 4 == 3) {
                vga_putchar(' ');
            }
        }

        // ASCII representation
        vga_puts("|");
        for (int i = 0; i < 16; i++) {
            if (current + i >= end) break;
            uint8_t c = current[i];
            vga_putchar((c >= 32 && c <= 126) ? c : '.');
        }
        vga_puts("|\n");

        current += 16;
    }

    vga_set_color(original_color & 0x0F, (original_color >> 4) & 0x0F);
}

// Safe memory test function
static void safe_memory_test(void) {
    // Use stack memory for testing
    uint8_t test_buffer[64];
    for (int i = 0; i < sizeof(test_buffer); i++) {
        test_buffer[i] = i;
    }
    
    vga_puts("\nDumping safe stack buffer:\n");
    hexdump((uint32_t*)test_buffer, sizeof(test_buffer));
}

int hexdump_command(const char *args) {
    // 1. Dump known-safe VGA memory (first 128 bytes only)
    vga_puts("Hexdump of VGA memory (first 128 bytes):\n");
    hexdump((uint32_t*)0xB8000, 128);

    // 2. Dump safe stack-based memory
    safe_memory_test();

    // 3. Skip physical page allocation until virtual memory is implemented
    vga_puts("\nPhysical page dumping disabled until VMM mapping is implemented\n");
    
    return 0;
}
