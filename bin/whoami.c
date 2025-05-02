// whoami_command.c
#include "../include/shell/shell.h"
#include "../include/video/vga.h"

void whoami_command(const char *args) {
    (void)args; // Unused parameter
    vga_puts("root\n");
}
