#include "../include/shell/shell.h"
#include "../include/video/vga.h"
#include "../include/lib/string.h"

void echo_command(const char *args) {
    // Check if arguments are NULL or empty
    if (args == NULL || *args == '\0') {
        vga_puts("echo: missing argument\nUsage: echo <text>\n");
        return;
    }

    // Skip leading whitespace
    while (*args == ' ') args++;

    // Check if the entire input is whitespace
    if (*args == '\0') {
        vga_putchar('\n');
        return;
    }

    // Handle quoted arguments (both single and double quotes)
    if (*args == '\'' || *args == '"') {
        char quote = *args;
        args++; // Move past the opening quote

        // Print characters until the closing quote is found
        while (*args != '\0' && *args != quote) {
            vga_putchar(*args); // Use vga_putchar instead of vga_putc
            args++;
        }

        // If the closing quote is missing, print an error
        if (*args != quote) {
            vga_puts("\necho: missing closing quote\n");
            return;
        }

        args++; // Move past the closing quote
    } else {
        // Print the rest of the arguments as-is
        while (*args != '\0') {
            vga_putchar(*args); // Use vga_putchar instead of vga_putc
            args++;
        }
    }

    vga_putchar('\n'); // Use vga_putchar for newline
}
