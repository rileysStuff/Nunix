#include "../include/shell/shell.h"
#include "../include/video/vga.h"

extern Command commands[];

void help_command(const char *args) {
    vga_puts("Available commands:\n");
    for (const Command *cmd = commands; cmd->name != NULL; cmd++) {
        vga_puts("  ");
        vga_puts(cmd->name); // Print command name
        vga_puts(" - ");     // Add separator
        vga_puts(cmd->description); // Print command description
        vga_puts("\n");
    }
}
