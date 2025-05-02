// cowsay_command
#include "../include/shell/shell.h"
#include "../include/video/vga.h"
#include "../include/lib/string.h"

char cowsays[100] = "\n\n"
                    "    ^__^\n"
                    "    (00)_______\n"
                    "    (__)       )>\n"
                    "       II---w II\n"
                    "       II     II\n";

void cowsay_command(const char *args) {
    int arg_len = 0;
    
    // Check if arguments are NULL or empty
    if (args == NULL || *args == '\0') {
        vga_puts("cowsay: missing argument\nUsage: cowsay <text>\n");
        return;
    }

    // Skip leading whitespace
    while (*args == ' ') args++;

    // Check if the entire input is whitespace
    if (*args == '\0') {
        vga_putchar('\n');
        return;
    }

    vga_puts("< ");
    
    // Handle quoted arguments (both single and double quotes)
    char quote = '\0';
    if (*args == '\'' || *args == '"') {
        quote = *args;
        args++; // Move past the opening quote
    }
    
    // Print characters until the closing quote or end of string
    while (*args != '\0' && (quote ? *args != quote : 1)) {
        vga_putchar(*args);
        arg_len++;
        args++;
    }

    // If closing quote is missing
    if (quote && *args != quote) {
        vga_puts("\necho: missing closing quote\n");
        return;
    }

    args++; // Move past the closing quote, if present
    vga_puts(" >\n");

    // Add a corresponding amount of dashes(-)
    for (int i = 0; i < arg_len + 4; i++) {
        vga_putchar('-');
    }

    vga_puts("\n");
    vga_puts(cowsays);
    vga_putchar('\n');
}
