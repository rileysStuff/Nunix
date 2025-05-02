#include "../include/kernel/panic/panic.h"

void panic_command(const char *args) {
    panic("Triggered kernel panic");
}
