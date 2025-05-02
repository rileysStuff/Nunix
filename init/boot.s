section .multiboot
align 4
multiboot_header:
    dd 0x1BADB002
    dd 0x00000003
    dd -(0x1BADB002 + 0x00000003)

section .text
global _start
extern main

_start:
    mov [multiboot_info_ptr], ebx
    call main
    cli
    hlt

section .bss
align 4
global multiboot_info_ptr
multiboot_info_ptr: resd 1
