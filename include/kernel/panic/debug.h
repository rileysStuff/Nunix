#ifndef DEBUG_H
#define DEBUG_H

#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include "../../video/vga.h"

/**
 * @brief Debug subsystem for kernel-level logging and diagnostics
 * 
 * Provides configurable logging levels, colored output, and formatted printing
 */

// Debug level enumeration
typedef enum {
    DEBUG_LEVEL_NONE   = 0,
    DEBUG_LEVEL_ERROR  = 1,
    DEBUG_LEVEL_WARN   = 2,
    DEBUG_LEVEL_INFO   = 3,
    DEBUG_LEVEL_DEBUG  = 4,
    DEBUG_LEVEL_TRACE  = 5
} DebugLevel;

// Color scheme for debug levels
typedef enum {
    DEBUG_COLOR_SUCCESS = VGA_COLOR_GREEN,
    DEBUG_COLOR_ERROR   = VGA_COLOR_RED,
    DEBUG_COLOR_WARN    = VGA_COLOR_YELLOW,
    DEBUG_COLOR_INFO    = VGA_COLOR_LIGHT_BLUE,
    DEBUG_COLOR_DEBUG   = VGA_COLOR_LIGHT_GREY,
    DEBUG_COLOR_TRACE   = VGA_COLOR_DARK_GREY,
    DEBUG_COLOR_LABEL   = VGA_COLOR_WHITE
} DebugColor;

// Debug logging macros
#define DEBUG_INIT()         debug_initialize()
#define DEBUG_SUCCESS(...)   debug_print(DEBUG_LEVEL_INFO,   DEBUG_COLOR_SUCCESS, " OK ", __VA_ARGS__)
#define DEBUG_ERROR(...)     debug_print(DEBUG_LEVEL_ERROR,  DEBUG_COLOR_ERROR,   " !! ", __VA_ARGS__)
#define DEBUG_WARN(...)      debug_print(DEBUG_LEVEL_WARN,   DEBUG_COLOR_WARN,    " ?? ", __VA_ARGS__)
#define DEBUG_INFO(...)      debug_print(DEBUG_LEVEL_INFO,   DEBUG_COLOR_INFO,    " -- ", __VA_ARGS__)
#define DEBUG_DEBUG(...)     debug_print(DEBUG_LEVEL_DEBUG,  DEBUG_COLOR_DEBUG,   " ** ", __VA_ARGS__)
#define DEBUG_TRACE(...)     debug_print(DEBUG_LEVEL_TRACE,  DEBUG_COLOR_TRACE,   " >> ", __VA_ARGS__)

/**
 * @brief Initialize debug subsystem
 */
void debug_initialize(void);

/**
 * @brief Set current debug verbosity level
 * @param level Maximum level to display (from DebugLevel enum)
 */
void debug_set_level(DebugLevel level);

/**
 * @brief Internal formatted print function
 * @param level Debug level of the message
 * @param color Color to use for message prefix
 * @param prefix 4-character message prefix
 * @param fmt Format string (printf-style)
 * @param ... Variable arguments
 */
void debug_print(DebugLevel level, DebugColor color, const char* prefix, 
                const char* fmt, ...);

#endif // DEBUG_H
