#ifndef PORTS_H
#define PORTS_H

#include <stdint.h>

/**
 * @file ports.h
 * @brief I/O port operations for x86 architecture
 */

/**
 * @brief Read a byte from an I/O port
 * @param port The I/O port to read from
 * @return The byte read from the port
 */
static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" 
                     : "=a"(ret) 
                     : "Nd"(port)
                     : "memory");
    return ret;
}

/**
 * @brief Write a byte to an I/O port
 * @param port The I/O port to write to
 * @param val The byte value to write
 */
static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" 
                     : 
                     : "a"(val), "Nd"(port)
                     : "memory");
}

/**
 * @brief Read a word (16 bits) from an I/O port
 * @param port The I/O port to read from
 * @return The word read from the port
 */
static inline uint16_t inw(uint16_t port) {
    uint16_t ret;
    __asm__ volatile ("inw %1, %0" 
                     : "=a"(ret) 
                     : "Nd"(port)
                     : "memory");
    return ret;
}

/**
 * @brief Write a word (16 bits) to an I/O port
 * @param port The I/O port to write to
 * @param val The word value to write
 */
static inline void outw(uint16_t port, uint16_t val) {
    __asm__ volatile ("outw %0, %1" 
                     : 
                     : "a"(val), "Nd"(port)
                     : "memory");
}

/**
 * @brief Read a double word (32 bits) from an I/O port
 * @param port The I/O port to read from
 * @return The double word read from the port
 */
static inline uint32_t inl(uint16_t port) {
    uint32_t ret;
    __asm__ volatile ("inl %1, %0" 
                     : "=a"(ret) 
                     : "Nd"(port)
                     : "memory");
    return ret;
}

/**
 * @brief Write a double word (32 bits) to an I/O port
 * @param port The I/O port to write to
 * @param val The double word value to write
 */
static inline void outl(uint16_t port, uint32_t val) {
    __asm__ volatile ("outl %0, %1" 
                     : 
                     : "a"(val), "Nd"(port)
                     : "memory");
}

#endif /* PORTS_H */
