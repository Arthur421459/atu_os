#include <atuos/time.h>
void sleep(uint32_t sec) {
    asm volatile ("int $0xA7" :: "a"(2), "b"(sec) : "memory", "cc");
}
void usleep(uint32_t usec) {
    asm volatile ("int $0xA7" :: "a"(0x82), "b"(usec) : "memory", "cc");
}