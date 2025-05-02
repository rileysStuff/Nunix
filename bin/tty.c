// bin/tty.c - Enhanced terminal name command
#include <video/vga.h>
#include <string.h>
#include <version/version.h>

int tty_command(int argc, char **argv) {
    int silent = 0;
    const char *tty_name = "tty0";
    
    for (int i = 1; i < argc; i++) {
        const char *arg = argv[i];
        if (strcmp(arg, "--help") == 0) {
            vga_puts("Usage: tty [OPTION]\n");
            vga_puts("Print terminal name.\n\nOptions:\n");
            vga_puts("  -s, --silent   Silent mode\n");
            vga_puts("  --help         Show help\n");
            vga_puts("  --version      Show version\n");
            return 0;
        } else if (strcmp(arg, "--version") == 0) {
            vga_puts("tty (Nunix) " NUNIX_VERSION "\n");
            return 0;
        } else if (strcmp(arg, "-s") == 0 || strcmp(arg, "--silent") == 0) {
            silent = 1;
        } else {
            vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
            vga_puts("tty: invalid option '");
            vga_puts(arg);
            vga_puts("'\n");
            return 1;
        }
    }

    if (!silent) {
        vga_puts(tty_name);
        vga_putchar('\n');
    }
    return 0; // Assuming TTY is always connected
}
