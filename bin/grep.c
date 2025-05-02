#include "../include/shell/shell.h"
#include "../include/video/vga.h"
#include "../include/keyboard/kb.h"
#include <string.h>

void grep_command(const char *args) {
    if (args == NULL || *args == '\0') {
        vga_puts("grep: missing pattern\nUsage: grep PATTERN\n");
        return;
    }

    // Skip leading whitespace in pattern
    while (*args == ' ') args++;
    const char *pattern = args;

    char line[256] = {0};
    int pos = 0;
    bool active = true;

    vga_puts("Interactive grep (press ESC to exit)\n");
    vga_puts("----------------------------------\n");

    while (active) {
        char c = kb_getchar();
        
        // Echo the character to screen
        if (c >= 32 && c <= 126) {  // Printable ASCII
            if (pos < sizeof(line)-1) {
                vga_putchar(c);
                line[pos++] = c;
            }
        }
        else if (c == '\b') {  // Backspace
            if (pos > 0) {
                vga_putchar('\b');
                vga_putchar(' ');
                vga_putchar('\b');
                pos--;
            }
        }
        else if (c == '\n') {  // Enter
            vga_putchar('\n');
            line[pos] = '\0';
            
            // Check for match
            if (strstr(line, pattern) != NULL) {
                vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
                vga_puts("MATCH: ");
                vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
                vga_puts(line);
                vga_putchar('\n');
            }
            
            // Reset for next line
            pos = 0;
            memset(line, 0, sizeof(line));
        }
        else if (c == 0x1B) {  // ESC key
            active = false;
        }
    }
    vga_puts("----------------------------------\n");
    vga_puts("Exited grep\n");
}
