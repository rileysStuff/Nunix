// bin/uname.c - Enhanced System information command
#include "../include/video/vga.h"
#include "../include/lib/string.h"
#include "../include/version/version.h"

static void print_usage() {
    vga_puts("Usage: uname [OPTION]...\n");
    vga_puts("Print system information.\n\n");
    vga_puts("Options:\n");
    vga_puts("  -a     Print all information\n");
    vga_puts("  -s     Print kernel name\n");
    vga_puts("  -n     Print network node hostname\n");
    vga_puts("  -r     Print kernel release\n");
    vga_puts("  -m     Print machine architecture\n");
    vga_puts("  -v     Print kernel build version\n");
    vga_puts("  -o     Print operating system\n");
    vga_puts("  --help Display this help\n");
}

void uname_command(const char *args) {
    char flags[20] = {0};
    int flag_count = 0;
    int print_all = 0;
    
    // Default behavior when no arguments
    if (!args || !*args) {
        vga_puts(BUNIX_NAME "\n");
        return;
    }

    if (args) {
        char *arg = strtok((char *)args, " ");
        while (arg) {
            if (strcmp(arg, "--help") == 0) {
                print_usage();
                return;
            } else if (arg[0] == '-') {
                for (int i = 1; arg[i]; i++) {
                    char c = arg[i];
                    if (c == 'a' || c == 's' || c == 'n' || 
                        c == 'r' || c == 'm' || c == 'v' || c == 'o') {
                        if (flag_count < 20) flags[flag_count++] = c;
                    } else {
                        vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
                        vga_puts("uname: invalid option '-");
                        vga_putchar(c);
                        vga_puts("'\n");
                        print_usage();
                        return;
                    }
                }
            } else {
                vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
                vga_puts("uname: unrecognized argument '");
                vga_puts(arg);
                vga_puts("'\n");
                print_usage();
                return;
            }
            arg = strtok(NULL, " ");
        }
    }

    for (int i = 0; i < flag_count; i++) {
        if (flags[i] == 'a') { print_all = 1; break; }
    }

    if (print_all || flag_count == 0) {
        vga_puts(NUNIX_NAME " " NUNIX_HOSTNAME " " NUNIX_VERSION " " 
                NUNIX_BUILD_VERSION " " NUNIX_ARCH " " NUNIX_OS "\n");
    } else {
    if (print_all) {
        vga_puts(BUNIX_NAME " " BUNIX_HOSTNAME " " BUNIX_VERSION " " 
                BUNIX_BUILD_VERSION " " BUNIX_ARCH " " BUNIX_OS "\n");
    } else if (flag_count > 0) {
        int first = 1;
        for (int i = 0; i < flag_count; i++) {
            if (!first) vga_putchar(' ');
            first = 0;
            switch(flags[i]) {
                case 's': vga_puts(NUNIX_NAME); break;
                case 'n': vga_puts(NUNIX_HOSTNAME); break;
                case 'r': vga_puts(NUNIX_VERSION); break;
                case 'm': vga_puts(NUNIX_ARCH); break;
                case 'v': vga_puts(NUNIX_BUILD_VERSION); break;
                case 'o': vga_puts(NUNIX_OS); break;
            }
        }
        vga_putchar('\n');
    }
}
}
