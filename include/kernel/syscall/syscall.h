#ifndef SYSCALL_H
#define SYSCALL_H

#include <stddef.h> // for size_t

// Syscall numbers
#define SYS_WRITE 4
#define SYS_EXIT 1

// Function prototype for sys_write
int sys_write(int fd, const void *buf, size_t count);
void sys_exit(int status);

#endif // SYSCALL_H
