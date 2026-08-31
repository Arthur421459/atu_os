#include <atuos/core.h>

void *malloc(size_t bytes) {
    void *pointer;
    asm volatile ("int $0xA7" : "=D"(pointer) : "a"(4), "b"(bytes) : "memory", "cc"); // pointer happy lol
    return pointer;
}
void print_wpos(const char* str, int pos) {
    asm volatile ("int $0xA7" :: "a"(1), "S"(str), "b"(pos) : "memory", "cc");
}
void free(void* ptr) {
    asm volatile ("int $0xA7" :: "D"(ptr), "a"(5));
}
void wait_irq(uint8_t irq) {
    asm volatile ("int $0xA7" :: "a"(3), "b"(irq) : "memory", "cc");
}