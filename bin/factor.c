#include "../include/shell/shell.h"
#include "../include/video/vga.h"
#include "../include/lib/string.h"

// Helper function to convert unsigned long to string
static void ul_to_str(unsigned long num, char *str) {
    char tmp[20];
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
    
    for (int j = 0; j < i; j++) {
        str[j] = tmp[i - j - 1];
    }
    str[i] = '\0';
}

void factor_command(const char *args) {
    // Check if arguments are NULL or empty
    if (args == NULL || *args == '\0') {
        vga_puts("factor: missing argument\nUsage: factor <number>\n");
        return;
    }

    // Skip leading whitespace
    while (*args == ' ') args++;

    // Check if the input is a valid number
    const char *ptr = args;
    while (*ptr) {
        if (*ptr < '0' || *ptr > '9') {
            vga_puts("factor: invalid number\n");
            return;
        }
        ptr++;
    }

    unsigned long num = 0;
    ptr = args;
    while (*ptr) {
        num = num * 10 + (*ptr - '0');
        if (num < (unsigned long)(*ptr - '0')) { // Check for overflow
            vga_puts("factor: number too large\n");
            return;
        }
        ptr++;
    }

    if (num < 2) {
        vga_puts("factor: number must be greater than 1\n");
        return;
    }

    // Print the original number
    vga_puts(args);
    vga_puts(": ");

    // Factor the number
    int first = 1;
    unsigned long n = num;
    char buf[20];

    // Check for 2 separately
    while (n % 2 == 0) {
        if (!first) vga_puts(" ");
        vga_puts("2");
        n /= 2;
        first = 0;
    }

    // Check odd divisors
    for (unsigned long i = 3; i <= n; i += 2) {
        while (n % i == 0) {
            if (!first) vga_puts(" ");
            ul_to_str(i, buf);
            vga_puts(buf);
            n /= i;
            first = 0;
        }
    }

   vga_putchar('\n');
}
