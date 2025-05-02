#include "../include/shell/shell.h"
#include "../include/video/vga.h"
#include "../include/kernel/rtc/rtc.h"

// Constants for weekday names
static const char *weekday_names[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

void date_command(const char *args) {
    // Ignore unused arguments
    (void)args;

    struct rtc_date date;
    rtc_read_full(&date); // Assuming this populates the `date` struct

    // Print the date in DD/MM/YYYY format
    vga_puts("Date: ");
    vga_putdec(date.day, 2);
    vga_putchar('/');
    vga_putdec(date.month, 2);
    vga_putchar('/');
    vga_putdec(date.year, 4);

    // Calculate the weekday using the function from rtc.c
    uint8_t weekday = day_of_week(date.day, date.month, date.year);

    // Print the weekday
    vga_puts(" (");
    if (weekday < 7) {
        vga_puts(weekday_names[weekday]);
    } else {
        vga_puts("???");
    }
    vga_puts(")\n");
}
