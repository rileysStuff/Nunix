ENTRY(_start)

SECTIONS {
    . = 1M; /* Kernel starts at 1MB */

    /* Multiboot header must be at the start */
    .multiboot : {
        *(.multiboot)
    }

    .text : {
        *(.text)
    }

    .rodata : {
        *(.rodata)
    }

    .data : {
        *(.data)
    }

    .bss : {
        *(COMMON)
        *(.bss)
    }

    /* Bitmap starts after the kernel */
    . = ALIGN(4K); /* Align to 4KB boundary */
    __bitmap_start = .; /* Symbol for bitmap location */
    . += 4K; /* Reserve 4KB for the bitmap (adjust as needed) */
}
