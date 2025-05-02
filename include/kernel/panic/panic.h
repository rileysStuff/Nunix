#ifndef PANIC_H
#define PANIC_H

#include <stdint.h>

// Only define the panic macro if it hasn't been defined elsewhere
#ifndef panic
__attribute__((noreturn)) void panic_impl(const char *file, int line, const char *message);
#define panic(msg) panic_impl(__FILE__, __LINE__, msg)
#endif

#endif // PANIC_H
