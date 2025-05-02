#include "../include/shell/shell.h"
#include "../include/video/vga.h"

// Xorshift32 PRNG (fast and decent randomness)
static unsigned int rand_state = 1;

unsigned int rand(void) {
    rand_state ^= rand_state << 13;
    rand_state ^= rand_state >> 17;
    rand_state ^= rand_state << 5;
    return rand_state;
}

// Convert unsigned int to string
static void uint_to_str(unsigned int num, char *str) {
    char tmp[16];
    int i = 0;
    
    if (num == 0) {
        str[0] = '0';
        str[1] = '\0';
        return;
    }
    
    while (num > 0) {
        tmp[i++] = '0' + (num % 10);
        num /= 10;
    }
    
    // Reverse digits
    int j = 0;
    while (i > 0) {
        str[j++] = tmp[--i];
    }
    str[j] = '\0';
}

void rand_command(const char *args) {
    unsigned int max = 32767; // Default: 0-32767 (like Bash's $RANDOM)
    char buf[16];

    // Parse argument (if provided)
    if (args && *args) {
        max = 0;
        while (*args >= '0' && *args <= '9') {
            max = max * 10 + (*args - '0');
            args++;
        }
        if (max == 0) max = 1; // Prevent division by zero
    }

    // Generate and print random number
    uint_to_str(rand() % max, buf);
    vga_puts(buf);
    vga_putchar('\n');
}
