#include "../../include/kernel/syscall/syscall.h"
#include <stddef.h>

// Implementation of the sys_write function
int sys_write(int fd, const void *buf, size_t count) {
    int ret;
    __asm__ volatile (
        "int $0x80" // Trigger interrupt 0x80 (syscall interrupt)
        : "=a" (ret) // Output in EAX (return value)
        : "a" (SYS_WRITE), // EAX = syscall number (SYS_WRITE)
          "b" (fd),        // EBX = file descriptor
          "c" (buf),       // ECX = buffer pointer
          "d" (count)      // EDX = count of bytes to write
        : "memory"        // Clobbered memory (buffer is being accessed)
    );
    return ret;
}

// Implementation of the sys_exit function
void sys_exit(int status) {
    __asm__ volatile (
        "int $0x80" // Trigger interrupt 0x80 (syscall interrupt)
        :
        : "a" (SYS_EXIT), // EAX = syscall number (SYS_EXIT)
          "b" (status)     // EBX = exit status
        : "memory"        // Clobbered memory
    );
}
