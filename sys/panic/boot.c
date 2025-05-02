/**
 *
 * Kernel Boot Sequence - Nunix OS
 * 
 * Performs hardware initialization and system diagnostics
 */

#include "../../include/kernel/panic/boot.h"
#include "../../include/video/vga.h"
#include "../../include/boot/multiboot.h"
#include "../../include/mm/vmm.h"
#include "../../include/kernel/panic/panic.h"
#include "../../include/keyboard/kb.h"
#include "../../include/shell/shell.h"
#include "../../include/kernel/panic/debug.h"
#include "../../include/kernel/arch/x86/cpu.h"

// Boot timing configuration
#define BOOT_DELAY_SHORT    150000
#define BOOT_DELAY_LONG     200000

extern uint32_t multiboot_info_ptr;
extern uint32_t __bitmap_start;

static void boot_delay(uint32_t milliseconds) {
    for (uint32_t i = 0; i < milliseconds * 1000; i++) {
        __asm__ volatile ("nop");
    }
}

static void verify_cpu_features(void) {
    DEBUG_INFO("Starting CPU verification...");
    
    // Test CPUID instruction
    uint32_t eax, ebx, ecx, edx;
    cpu_cpuid(0, 0, &eax, &ebx, &ecx, &edx);
    
    if (eax == 0 && ebx == 0 && ecx == 0 && edx == 0) {
        panic("PANIC! CPUID instruction verification failed");
    }
    DEBUG_SUCCESS("CPUID instruction verified");

    // Test register integrity
    const uint32_t test_value = 0x12345678;
    uint32_t read_value;
    
    __asm__ volatile (
        "mov %1, %%eax\n\t"
        "mov %%eax, %0\n\t"
        : "=r" (read_value)
        : "r" (test_value)
        : "%eax"
    );

    if (read_value != test_value) {
        panic("PANIC! CPU register verification failed");
    }
    DEBUG_SUCCESS("Register integrity verified");

    // Check TSC functionality
    cpu_info_t cpu_info;
    cpu_identify(&cpu_info);
    
    if (cpu_info.features.tsc) {
        const uint64_t tsc1 = cpu_rdtsc();
        boot_delay(1000);
        const uint64_t tsc2 = cpu_rdtsc();
        
        if (tsc2 <= tsc1) {
            panic("PANIC! TSC counter verification failed");
        }
        DEBUG_SUCCESS("TSC counter operational");
    }
}

void boot_screen(void) {
    DEBUG_INIT();
    DEBUG_INFO("Starting kernel boot sequence");

    // Initialize VGA with explicit black background
    vga_initialize();
    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    vga_clear();
    DEBUG_SUCCESS("Video subsystem initialized");
    boot_delay(BOOT_DELAY_SHORT);

    // Early CPU initialization
    cpu_early_init();
    verify_cpu_features();

    // Memory management initialization
    DEBUG_INFO("Initializing virtual memory manager");
    const struct multiboot_info* mb_info = (struct multiboot_info*)multiboot_info_ptr;
    const uint64_t total_memory = (mb_info->mem_upper + 1024) * 1024;
    
    vmm_init((uint32_t*)&__bitmap_start, total_memory);
    DEBUG_SUCCESS("Memory manager initialized (%u MB)", 
                 total_memory / (1024 * 1024));
    boot_delay(BOOT_DELAY_SHORT);

    // Peripheral initialization
    if (kb_init() != 0) {
        panic("PANIC! Keyboard controller initialization failed");
    }
    DEBUG_SUCCESS("Input subsystem initialized");
    boot_delay(BOOT_DELAY_SHORT);

    // Final system checks
    DEBUG_INFO("Performing final system checks");
    DEBUG_SUCCESS("Kernel base address: 0xC0000000");
    
    cpu_info_t cpu_info;
    cpu_identify(&cpu_info);
    DEBUG_INFO("CPU Vendor: %s", cpu_info.identity.vendor_id);
    DEBUG_INFO("CPU Model:  %s", cpu_info.identity.brand_string);

    // Complete initialization
    cpu_late_init();
    kb_set_boot_complete(true);
    
    // Final clear with black background
    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    vga_clear();
    boot_delay(BOOT_DELAY_LONG);
    
    // Enable keyboard input LAST
    kb_enable_input(true);
}
