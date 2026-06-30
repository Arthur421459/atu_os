#include "atuos/core.h"
void wait_irq(uint8_t irq) {
    asm ("int $0xA7" :: "a"(3), "b"(irq) : "memory", "cc");
}