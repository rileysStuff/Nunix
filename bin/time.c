#include "../include/shell/shell.h"
#include "../include/video/vga.h"
#include "../include/kernel/rtc/rtc.h"
#include "../include/lib/string.h"

// Function to display time in a formatted way
void display_time(const struct rtc_date *date, const char *label) {
    vga_puts(label);
    vga_putchar(' ');
    vga_putdec(date->hour, 2);
    vga_putchar(':');
    vga_putdec(date->minute, 2);
    vga_putchar(':');
    vga_putdec(date->second, 2);

    if (!date->is_24hour) {
        vga_puts(date->is_pm ? " PM" : " AM");
    }
    vga_puts("\n");
}

// Function to check if the given timezone is valid
int is_valid_timezone(const char *timezone) {
    const char *valid_timezones[] = {"de", "us", "jp", "in", "uk"};
    for (int i = 0; i < sizeof(valid_timezones) / sizeof(valid_timezones[0]); i++) {
        if (strcmp(timezone, valid_timezones[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

// Function to get the timezone offset and label
void get_timezone_info(const char *timezone, struct rtc_date *date, int *offset, const char **label) {
    if (strcmp(timezone, "de") == 0) {
        // Germany timezone (CET/CEST)
        *offset = is_dst_eu(date->month, date->day, date->year) ? 2 : 1;
        *label = is_dst_eu(date->month, date->day, date->year) ? "CEST Time:" : "CET Time:";
    } else if (strcmp(timezone, "us") == 0) {
        // US Eastern timezone (EST/EDT)
        *offset = is_dst_us(date->month, date->day, date->year) ? -4 : -5;
        *label = is_dst_us(date->month, date->day, date->year) ? "EDT Time:" : "EST Time:";
    } else if (strcmp(timezone, "jp") == 0) {
        // Japan timezone (JST)
        *offset = 9;
        *label = "JST Time:";
    } else if (strcmp(timezone, "in") == 0) {
        // India timezone (IST)
        *offset = 5;
        *label = "IST Time:";
    } else if (strcmp(timezone, "uk") == 0) {
        // UK timezone (GMT/BST)
        *offset = is_dst_eu(date->month, date->day, date->year) ? 1 : 0;
        *label = is_dst_eu(date->month, date->day, date->year) ? "BST Time:" : "GMT Time:";
    }
}

// Main time command function
void time_command(const char *args) {
    struct rtc_date date;
    rtc_read_full(&date);

    // Display UTC time
    display_time(&date, "UTC Time:");

    // Handle timezone arguments
    if (args != NULL) {
        if (!is_valid_timezone(args)) {
            vga_puts("Invalid timezone. Use 'de' for Germany, 'us' for US, 'jp' for Japan, 'in' for India, or 'uk' for UK\n");
            return;
        }

        struct rtc_date local = date;
        int offset;
        const char *label;

        get_timezone_info(args, &local, &offset, &label);
        rtc_adjust_timezone(&local, offset);
        display_time(&local, label);
    }
}
