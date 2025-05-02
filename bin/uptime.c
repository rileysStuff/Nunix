#include "../include/kernel/rtc/rtc.h"
#include "../include/video/vga.h"
#include <stdint.h>
#include <stdbool.h>

// Global variable to store the boot time
static struct rtc_date boot_time;

// Function to initialize the boot time
void uptime_init() {
    rtc_read_full(&boot_time); // Read the boot time from RTC
}

// Function to calculate seconds since epoch for a given rtc_date
static uint32_t rtc_date_to_seconds(const struct rtc_date *date) {
    uint32_t seconds = 0;

    // Add years (2000-2099)
    for (uint16_t y = 2000; y < 2000 + date->year; y++) {
        seconds += is_leap_year(y - 2000) ? 31622400 : 31536000; // 366 or 365 days
    }

    // Add months
    for (uint8_t m = 1; m < date->month; m++) {
        seconds += days_in_month(m, date->year) * 86400;
    }

    // Add days, hours, minutes, and seconds
    seconds += (date->day - 1) * 86400;
    seconds += date->hour * 3600;
    seconds += date->minute * 60;
    seconds += date->second;

    return seconds;
}

// Function to calculate uptime in seconds
uint32_t calculate_uptime() {
    struct rtc_date current_time;
    rtc_read_full(&current_time); // Read the current time from RTC

    // Convert boot time and current time to seconds since epoch
    uint32_t boot_seconds = rtc_date_to_seconds(&boot_time);
    uint32_t current_seconds = rtc_date_to_seconds(&current_time);

    // Calculate uptime
    return current_seconds - boot_seconds;
}

// Function to display uptime
void uptime_command(const char *args) {
    uint32_t uptime_seconds = calculate_uptime();

    // Convert uptime to days, hours, minutes, and seconds
    uint32_t days = uptime_seconds / 86400;
    uptime_seconds %= 86400;
    uint32_t hours = uptime_seconds / 3600;
    uptime_seconds %= 3600;
    uint32_t minutes = uptime_seconds / 60;
    uptime_seconds %= 60;

    // Display uptime
    vga_puts("Uptime: ");
    vga_putdec(days, 2);  // Display days with 2 digits
    vga_puts(" days, ");
    vga_putdec(hours, 2);  // Display hours with 2 digits
    vga_puts(" hours, ");
    vga_putdec(minutes, 2);  // Display minutes with 2 digits
    vga_puts(" minutes, ");
    vga_putdec(uptime_seconds, 2);  // Display seconds with 2 digits
    vga_puts(" seconds\n");
}
