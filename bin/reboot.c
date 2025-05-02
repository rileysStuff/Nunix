#include "../include/shell/shell.h"
#include "../include/video/vga.h"
#include <stdint.h>

// Small delay function for IO operations
static void io_wait(void) {
    // This wastes about 1-4 microseconds on most machines
    __asm__ volatile ("outb %%al, $0x80" : : "a"(0));
}

// Function to force a system reset by causing a triple fault
static void system_fallback_reset(void) {
    vga_puts("Attempting triple fault reset...\n");
    
    // Disable interrupts
    __asm__ volatile ("cli");
    
    // Create a null IDT descriptor
    uint8_t idt_descriptor[6] = {0};
    
    // Load the null IDT descriptor
    __asm__ volatile ("lidt (%0)" : : "r" (idt_descriptor) : "memory");
    
    // Trigger a triple fault by causing an exception
    __asm__ volatile ("int $0");
    
    // If we're still here, try to divide by zero
    volatile int zero = 0;
    __asm__ volatile ("div %0" : : "r"(zero));
    
    // Final fallback - just halt forever
    for(;;) {
        __asm__ volatile ("hlt");
    }
}

// Function to attempt a system reboot via 8042 keyboard controller
static void try_8042_reset(void) {
    vga_puts("Attempting 8042 controller reset...\n");
    
    uint8_t status;
    int timeout = 500000; // Increased timeout
    
    // Disable interrupts during reset attempt
    __asm__ volatile ("cli");
    
    // Wait for the 8042 controller to be ready
    do {
        __asm__ volatile ("inb $0x64, %0" : "=a"(status));
        io_wait();
        if (--timeout == 0) {
            vga_puts("8042 controller not responding.\n");
            return;
        }
    } while (status & 0x02);
    
    // Send the reset command
    __asm__ volatile ("outb %0, $0x64" : : "a"((uint8_t)0xFE));
    io_wait();
    
    vga_puts("Reset command sent to 8042 controller.\n");
    
    // Give some time for reset to occur
    for (timeout = 0; timeout < 1000000; timeout++) {
        io_wait();
    }
}

// Main reboot command
void reboot_command(const char *args) {
    (void)args; // Unused parameter
    
    vga_puts("Initiating system reboot...\n");
    
    // First try the proper 8042 method
    try_8042_reset();
    
    // If we're still here, try fallback methods
    vga_puts("Primary reset failed, trying fallback...\n");
    system_fallback_reset();
    
    // If we're STILL here, something is very wrong
    vga_puts("All reboot methods failed! System halted.\n");
    for(;;) {
        __asm__ volatile ("hlt");
    }
}
