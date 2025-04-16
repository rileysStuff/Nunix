/**
 * Nunix Kernel - Main Entry Point
 * 
 * Responsible for system initialization and core component coordination.
 * 
 * © 2025 Nunix OS Developers. All Rights Reserved.
 */

#include "../include/boot/multiboot.h"
#include "../include/kernel/panic/panic.h"
#include "../include/kernel/panic/boot.h"
#include "../include/kernel/rtc/rtc.h"
#include "../include/keyboard/kb.h"
#include "../include/mm/vmm.h"
#include "../include/shell/shell.h"
#include "../include/version/version.h"
#include "../include/video/vga.h"
#include <stdio.h>

/*─────────────────────────────────────────────────────────────────────────────*
 * MULTIBOOT HEADER (Must reside in first 8KB of kernel image)                *
 *─────────────────────────────────────────────────────────────────────────────*/
__attribute__((section(".multiboot"), aligned(4)))
static const struct multiboot_header multiboot_header = {
    .magic     = MULTIBOOT_HEADER_MAGIC,
    .flags     = MULTIBOOT_HEADER_FLAGS,
    .checksum  = -(MULTIBOOT_HEADER_MAGIC + MULTIBOOT_HEADER_FLAGS)
};

/*─────────────────────────────────────────────────────────────────────────────*
 * EXTERN DECLARATIONS                                                         *
 *─────────────────────────────────────────────────────────────────────────────*/
extern uint32_t multiboot_info_ptr;  /* Multiboot info structure pointer      */
extern uint32_t __bitmap_start;      /* Virtual memory bitmap start address   */

/*─────────────────────────────────────────────────────────────────────────────*
 * SYSTEM CONTROL FUNCTIONS                                                    *
 *─────────────────────────────────────────────────────────────────────────────*/

/**
 * kernel_halt - Gracefully halt system execution
 * 
 * Disables interrupts and enters an indefinite wait state. This function
 * does not return.
 */
static void kernel_halt(void) 
{
    __asm__ volatile ("cli");      /* Disable interrupts */
    for (;;) {
        __asm__ volatile ("hlt");  /* Wait for next interrupt */
    }
}

/**
 * crude_delay - Simple blocking delay implementation
 * @milliseconds: Approximate delay duration in milliseconds
 * 
 * Note: Temporary implementation until proper timer subsystem is available
 */
static void crude_delay(uint32_t milliseconds) 
{
    /* Approximately 1 nop per microsecond (adjusted for current clock speed) */
    for (uint32_t i = 0; i < milliseconds * 1000; ++i) {
        __asm__ volatile ("nop");
    }
}

/*─────────────────────────────────────────────────────────────────────────────*
 * USER INTERFACE COMPONENTS                                                   *
 *─────────────────────────────────────────────────────────────────────────────*/

/**
 * display_system_banner - Present formatted system information
 * 
 * Shows version information, build details, and copyright notice in a
 * consistent, professional layout.
 */

static void display_system_banner(void) 
{
    struct rtc_date current_date;
    rtc_read_full(&current_date);
    
    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);

    /* System Identity Art */
    vga_puts(
        "\n"
        "+------------------------------------+\n"
        "|  _   _             _               |\n"
        "| | \\ | |_   _ _ __ (_)_  __         |\n"
        "| |  \\| | | | | '_ \\| \\ \\/ /         |\n"
        "| |  \\  | |_| | | | | |>  <          |\n"
        "| |_| \\_|\\__,_|_| |_|_/_/\\_\\         |\n"
        "+------------------------------------+\n"
    );    

    /* System Information */
    vga_puts("\n  Version:      ");
    vga_puts(NUNIX_VERSION);
    vga_puts("\n");

    vga_puts("  Build:        ");
    vga_puts(__DATE__);
    vga_puts(" ");
    vga_puts(__TIME__);
    vga_puts("\n");

    vga_puts("  Copyright:    20");
    vga_putchar('0' + ((current_date.year % 100) / 10));
    vga_putchar('0' + (current_date.year % 10));
    vga_puts("-");
    if (current_date.month < 10) vga_putchar('0');
    vga_putdec(current_date.month, 0);
    vga_puts("-");
    if (current_date.day < 10) vga_putchar('0');
    vga_putdec(current_date.day, 0);
    vga_puts(" Nunix OS\n");

    /* Informative Separator */
    vga_puts("\n  ---------------------------------\n\n");

    /* User Guidance */
    vga_puts("  [github.com/rileysStuff/Nunix], forked from: [github.com/0x16000/Bunix]        \n");
    vga_puts("  Type 'help' to list all available commands  \n\n");
}

/*─────────────────────────────────────────────────────────────────────────────*
 * MAIN KERNEL ENTRY POINT                                                     *
 *─────────────────────────────────────────────────────────────────────────────*/

/**
 * kernel_main - Primary system initialization and control flow
 * 
 * Orchestrates boot sequence, hardware initialization, and user interface
 * presentation before transferring control to the shell.
 */
int main(void) 
{
    /* Boot Sequence */
    boot_screen();
    crude_delay(500);  /* Allow boot screen visibility */

    /* System Initialization */
    display_system_banner();
    kb_enable_input(true);

    /* User Environment */
    print_shell_prompt();
    shell_run();

    /* Unreachable Code Handling */
    panic("PANIC! Kernel shell terminated abnormally");
    
    return 0;  /* Satisfy compiler requirement */
}
