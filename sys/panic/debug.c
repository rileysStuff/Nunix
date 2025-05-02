/**
 * Kernel debug implementation
 * 
 * Provides formatted output capabilities for system diagnostics
 */

#include "../../include/kernel/panic/debug.h"
#include "../../include/video/vga.h"
#include "../../include/kernel/ports/ports.h"
#include <stdarg.h>
#include <stdbool.h>

// Module configuration
#define DEBUG_BUFFER_SIZE   256
#define HEX_PREFIX         "0x"
#define HEX_DIGITS         8

static DebugLevel current_debug_level = DEBUG_LEVEL_INFO;

// Internal formatting functions
static size_t format_decimal(char* buffer, int number);
static size_t format_hex(char* buffer, unsigned int number);
static size_t format_string(char* buffer, const char* string);

void debug_initialize(void) {
    vga_initialize();
    current_debug_level = DEBUG_LEVEL_INFO;
    DEBUG_INFO("Debug subsystem initialized [Level: %d]", current_debug_level);
}

void debug_set_level(DebugLevel level) {
    current_debug_level = level;
    DEBUG_INFO("Debug verbosity level changed to %d", level);
}

void debug_print(DebugLevel level, DebugColor color, const char* prefix, 
                const char* fmt, ...) {
    if (level > current_debug_level) return;

    // Save original color state
    const uint8_t original_color = vga_get_color();
    
    // Print formatted prefix
    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    vga_puts("[ ");

    vga_set_color(color, VGA_COLOR_BLACK);
    vga_puts(prefix);

    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    vga_puts(" ] ");

    // Process format string
    va_list args;
    va_start(args, fmt);
    
    char buffer[DEBUG_BUFFER_SIZE];
    char* buffer_ptr = buffer;
    
    for (const char* fmt_ptr = fmt; *fmt_ptr; fmt_ptr++) {
        if (*fmt_ptr != '%') {
            *buffer_ptr++ = *fmt_ptr;
            continue;
        }

        // Handle format specifier
        switch (*++fmt_ptr) {
            case 'd': {
                const int num = va_arg(args, int);
                buffer_ptr += format_decimal(buffer_ptr, num);
                break;
            }
            
            case 'x': {
                const unsigned int num = va_arg(args, unsigned int);
                buffer_ptr += format_hex(buffer_ptr, num);
                break;
            }
            
            case 's': {
                const char* str = va_arg(args, char*);
                buffer_ptr += format_string(buffer_ptr, str);
                break;
            }
            
            case 'c': {
                const char c = (char)va_arg(args, int);
                *buffer_ptr++ = c;
                break;
            }
            
            default:
                *buffer_ptr++ = *fmt_ptr;
                break;
        }
    }
    
    *buffer_ptr = '\0';
    va_end(args);

    // Output formatted message
    vga_set_color(DEBUG_COLOR_LABEL, VGA_COLOR_BLACK);
    vga_puts(buffer);
    vga_putchar('\n');

    // Restore original color state
    vga_set_color(original_color >> 4, original_color & 0x0F);
}

// Implementation of formatting helpers
static size_t format_decimal(char* buffer, int number) {
    if (number == 0) {
        *buffer = '0';
        return 1;
    }

    char temp[16];
    char* ptr = temp;
    size_t length = 0;
    const bool is_negative = number < 0;

    if (is_negative) {
        number = -number;
    }

    while (number > 0) {
        *ptr++ = '0' + (number % 10);
        number /= 10;
        length++;
    }

    if (is_negative) {
        *buffer++ = '-';
        length++;
    }

    while (ptr > temp) {
        *buffer++ = *--ptr;
    }

    return length;
}

static size_t format_hex(char* buffer, unsigned int number) {
    const char hex_digits[] = "0123456789ABCDEF";
    size_t length = 0;

    // Add hexadecimal prefix
    *buffer++ = '0';
    *buffer++ = 'x';
    length += 2;

    // Format 8-digit hexadecimal
    for (int shift = 28; shift >= 0; shift -= 4) {
        const uint8_t nibble = (number >> shift) & 0xF;
        *buffer++ = hex_digits[nibble];
        length++;
    }

    return length;
}

static size_t format_string(char* buffer, const char* string) {
    size_t length = 0;
    
    while (*string) {
        *buffer++ = *string++;
        length++;
    }
    
    return length;
}
