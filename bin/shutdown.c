#include "../include/shell/shell.h"
#include "../include/video/vga.h"
#include <stdint.h>
#include <stdbool.h>

// Delay function using a simple loop
static void delay(uint32_t count) {
    while(count--) {
        __asm__ volatile("nop");
    }
}

/**
 * Attempt ACPI shutdown via the 0x604 port
 * Returns true if shutdown was initiated, false otherwise
 */
static bool try_acpi_shutdown() {
    // ACPI shutdown requires writing to specific ports
    for (int i = 0; i < 3; i++) {  // Try multiple times
        // Write shutdown command to ACPI PM1a control register
        __asm__ volatile (
            "outw %w0, %w1"
            : 
            : "a" ((uint16_t)0x2000), "Nd" ((uint16_t)0x604)
            : "memory"
        );
        
        // Wait a bit to see if shutdown occurs
        delay(10000);
        
        // If we're still here, shutdown didn't work
        vga_puts("ACPI shutdown attempt ");
        vga_putdec(i+1, 1);
        vga_puts(" failed\n");
    }
    return false;
}

/**
 * Attempt Bochs/QEMU shutdown via the 0xB004 port
 * This is specific to these emulators
 */
static bool try_bochs_shutdown() {
    vga_puts("Attempting Bochs/QEMU shutdown...\n");
    __asm__ volatile (
        "outw %w0, %w1"
        : 
        : "a" ((uint16_t)0x2000), "Nd" ((uint16_t)0xB004)
        : "memory"
    );
    
    // Wait to see if shutdown occurs
    delay(10000);
    return false; // If we're here, it didn't work
}

/**
 * Fallback reset function that forces a triple fault
 */
static void system_fallback_reset() {
    vga_puts("Triggering triple fault to force reset...\n");
    
    // Create a null IDT descriptor
    uint8_t idt_descriptor[6] = {0};
    
    // Load the null IDT
    __asm__ volatile (
        "lidt (%0)"
        :
        : "r" (idt_descriptor)
        : "memory"
    );
    
    // Trigger a divide by zero exception
    __asm__ volatile (
        "xor %%eax, %%eax\n\t"
        "div %%eax"  // Divide by zero
        :
        :
        : "eax", "memory"
    );
    
    // If we get here (which we shouldn't), halt
    __asm__ volatile ("hlt");
}

/**
 * Shutdown command entry point
 * @param args Unused command line arguments
 */
void shutdown_command(const char *args) {
    (void)args; // Explicitly mark unused parameter
    
    vga_puts("Attempting system shutdown...\n");
    
    // Try ACPI shutdown first
    if (try_acpi_shutdown()) {
        return; // Shouldn't actually return if successful
    }
    
    // Try emulator-specific shutdown
    if (try_bochs_shutdown()) {
        return; // Shouldn't actually return if successful
    }
    
    // If all else fails, force a reset
    vga_puts("All shutdown methods failed, forcing reset...\n");
    system_fallback_reset();
    
    // We should never get here
    __asm__ volatile ("hlt");
}
