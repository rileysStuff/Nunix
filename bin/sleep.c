#include "../include/shell/shell.h"
#include "../include/video/vga.h"
#include "../include/lib/string.h"
#include "../include/kernel/ports/ports.h"

// New calibrated value based on your observation
static void busy_delay(unsigned int seconds) {
    // ADJUSTED TO 2.5 BILLION LOOPS PER SECOND
    const unsigned long loops_per_second = 2500000000; // 2,500,000,000
    
    for (unsigned int i = 0; i < seconds; i++) {
        for (volatile unsigned long j = 0; j < loops_per_second; j++) {
            asm volatile ("nop");
        }
    }
}

void sleep_command(const char *args) {
    if (!args || !*args) {
        vga_puts("sleep: missing argument\nUsage: sleep <seconds>\n");
        return;
    }
    
    // Improved number parsing
    unsigned int seconds = 0;
    while (*args >= '0' && *args <= '9') {
        seconds = seconds * 10 + (*args - '0');
        args++;
    }
    
    if (seconds == 0) {
        vga_puts("sleep: time value must be positive\n");
        return;
    }
    
    busy_delay(seconds);
}
