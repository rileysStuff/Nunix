#ifndef _RTC_H
#define _RTC_H

#include <stdint.h>
#include <stdbool.h>

// RTC Register Ports
#define RTC_INDEX_PORT   0x70
#define RTC_DATA_PORT    0x71

// RTC Register Numbers
#define RTC_SECONDS      0x00
#define RTC_MINUTES      0x02
#define RTC_HOURS        0x04
#define RTC_DAY_OF_MONTH 0x07
#define RTC_MONTH        0x08
#define RTC_YEAR         0x09
#define RTC_STATUS_A     0x0A
#define RTC_STATUS_B     0x0B
#define RTC_STATUS_C     0x0C
#define RTC_STATUS_D     0x0D

// Status Register B Flags
#define RTC_24HOUR_MODE  0x02
#define RTC_BCD_MODE     0x04
#define RTC_UPDATE_IN_PROGRESS 0x80

// Date Structure
struct rtc_date {
    uint8_t second;
    uint8_t minute;
    uint8_t hour;
    uint8_t day;
    uint8_t month;
    uint16_t year;  // Last two digits (00-99)
    bool is_pm;
    bool is_24hour;
};

// Function Prototypes

// Basic RTC Operations
uint8_t rtc_read_register(uint8_t reg);
void rtc_write_register(uint8_t reg, uint8_t value);
bool rtc_is_updating();

// Date/Time Functions
void rtc_read_date(struct rtc_date *date);
void rtc_read_time(struct rtc_date *date);
void rtc_read_full(struct rtc_date *date);

// Conversion Functions
uint8_t bcd_to_bin(uint8_t bcd);
uint8_t bin_to_bcd(uint8_t bin);

// Date Calculations
bool is_leap_year(uint8_t year);
uint8_t days_in_month(uint8_t month, uint8_t year);
uint8_t day_of_week(uint8_t day, uint8_t month, uint16_t year);

// DST Calculations
bool is_dst_eu(uint8_t month, uint8_t day, uint8_t year);
bool is_dst_us(uint8_t month, uint8_t day, uint8_t year);

// Utility Functions
uint8_t rtc_calculate_last_sunday(uint8_t month, uint8_t year);
uint8_t rtc_calculate_nth_weekday(uint8_t month, uint8_t year, 
                                 uint8_t n, uint8_t weekday);

// Timezone Functions
void rtc_adjust_timezone(struct rtc_date *date, int8_t offset);

#endif // _RTC_H
