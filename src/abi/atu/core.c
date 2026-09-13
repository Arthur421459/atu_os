#include <atuos/core.h>
#include <atuos/syscall.h>
void *malloc(size_t bytes) {
    void *pointer;
    asm volatile ("int $0xA7" : "=D"(pointer) : "a"(4), "b"(bytes) : "memory", "cc"); // pointer happy lol
    return pointer;
}
void print_wpos(const char* str, int pos) {
    atuos_syscall(1, pos, 0, 0, (void*)str, 0);
}
void free(void* ptr) {
    asm volatile ("int $0xA7" :: "D"(ptr), "a"(5));
}
void wait_irq(uint8_t irq) {
    asm volatile ("int $0xA7" :: "a"(3), "b"(irq) : "memory", "cc");
}