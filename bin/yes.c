#include "../include/video/vga.h"
#include "../include/keyboard/kb.h"

void yes_command(const char *args) {
    const char *message = "y";
    if (args && args[0] != '\0') {
        message = args;
    }
    
    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    
    while(1) {
        vga_puts(message);
        vga_putchar('\n');
        
        // Check for ESC
        if (kb_check_escape() || (kb_ctrl_pressed() && kb_getchar() == 'c')) {
            vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
            vga_puts("\n[yes interrupted]\n");
            kb_flush();
            return;
        }
    }
}
