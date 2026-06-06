#ifndef IO_H
#define IO_H
#include <stdint.h>

static inline uint8_t inb(uint16_t port) {
    uint8_t result;
    asm volatile ("inb %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}
static inline uint16_t inw(uint16_t port) {
    uint16_t result;
    asm volatile ("inw %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}
static inline void outb(uint16_t port, uint8_t value) {
    asm volatile ("outb %0, %1" :: "a"(value), "Nd"(port));
}
static inline void outw(uint16_t port, uint16_t value) {
    asm volatile ("outw %0, %1" :: "a"(value), "Nd"(port));
}
#endif