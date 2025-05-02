#include "../include/shell/shell.h"
#include "../include/video/vga.h"

void clear_command(const char *args) {
    vga_clear();
}
