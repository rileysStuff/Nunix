#include "../include/shell/shell.h"
#include "../include/video/vga.h"
#include "../include/lib/string.h"

// Custom atoi implementation
int simple_atoi(const char *str) {
    int result = 0;
    int sign = 1;
    
    // Handle leading whitespace
    while (*str == ' ') str++;
    
    // Handle optional sign
    if (*str == '-') {
        sign = -1;
        str++;
    } else if (*str == '+') {
        str++;
    }
    
    // Convert digits to integer
    while (*str >= '0' && *str <= '9') {
        result = result * 10 + (*str - '0');
        str++;
    }
    
    return sign * result;
}

// Custom itoa implementation
void simple_itoa(int num, char *buffer) {
    int i = 0;
    int is_negative = 0;
    
    // Handle 0 explicitly
    if (num == 0) {
        buffer[i++] = '0';
        buffer[i] = '\0';
        return;
    }
    
    // Handle negative numbers
    if (num < 0) {
        is_negative = 1;
        num = -num;
    }
    
    // Process individual digits
    while (num != 0) {
        int rem = num % 10;
        buffer[i++] = rem + '0';
        num = num / 10;
    }
    
    // Add negative sign if needed
    if (is_negative) {
        buffer[i++] = '-';
    }
    
    buffer[i] = '\0';
    
    // Reverse the string
    int start = 0;
    int end = i - 1;
    while (start < end) {
        char temp = buffer[start];
        buffer[start] = buffer[end];
        buffer[end] = temp;
        start++;
        end--;
    }
}

void expr_command(const char *args) {
    if (args == NULL || *args == '\0') {
        vga_puts("expr: missing arguments\nUsage: expr NUM1 OP NUM2\n");
        return;
    }

    // Skip leading whitespace
    while (*args == ' ') args++;

    // Parse first number
    const char *num1_start = args;
    while (*args >= '0' && *args <= '9') args++;
    if (args == num1_start) {
        vga_puts("expr: invalid first number\n");
        return;
    }
    char num1_buf[32];
    strncpy(num1_buf, num1_start, args - num1_start);
    num1_buf[args - num1_start] = '\0';
    int num1 = simple_atoi(num1_buf);

    // Skip whitespace before operator
    while (*args == ' ') args++;

    // Parse operator
    if (*args == '\0') {
        vga_puts("expr: missing operator\n");
        return;
    }
    char op = *args++;
    
    // Skip whitespace after operator
    while (*args == ' ') args++;

    // Parse second number
    const char *num2_start = args;
    while (*args >= '0' && *args <= '9') args++;
    if (args == num2_start) {
        vga_puts("expr: invalid second number\n");
        return;
    }
    char num2_buf[32];
    strncpy(num2_buf, num2_start, args - num2_start);
    num2_buf[args - num2_start] = '\0';
    int num2 = simple_atoi(num2_buf);

    // Perform calculation
    int result;
    switch (op) {
        case '+':
            result = num1 + num2;
            break;
        case '-':
            result = num1 - num2;
            break;
        case '*':
            result = num1 * num2;
            break;
        case '/':
            if (num2 == 0) {
                vga_puts("expr: division by zero\n");
                return;
            }
            result = num1 / num2;
            break;
        case '%':
            if (num2 == 0) {
                vga_puts("expr: division by zero\n");
                return;
            }
            result = num1 % num2;
            break;
        case '=':
            result = (num1 == num2);
            break;
        case '!':
            if (*args++ != '=') {
                vga_puts("expr: unknown operator\n");
                return;
            }
            result = (num1 != num2);
            break;
        case '>':
            result = (num1 > num2);
            break;
        case '<':
            result = (num1 < num2);
            break;
        default:
            vga_puts("expr: unknown operator\n");
            return;
    }

    // Print result
    char buf[32];
    simple_itoa(result, buf);
    vga_puts(buf);
    vga_putchar('\n');
}
