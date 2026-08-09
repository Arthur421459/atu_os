#include "kernel/idt.h"
struct idt_entry idt[256];
struct idt_ptr itr;
void set_interrupt_idt(int i, uint32_t offset, uint8_t attributes, uint16_t selector) {
    idt[i].low_offset = offset & 0xFFFF;
    idt[i].high_offset = (offset >> 16) & 0xFFFF;
    
    idt[i].attributes = attributes;
    idt[i].selector = selector;

    idt[i].reserved_zero = 0; // nunca se sabe ne?
}
void config_idt() {
    itr.base = (uintptr_t)&idt;
    itr.limit = sizeof(idt) - 1;
    for (int i = 0; i < 256; i++) {
        switch (i) {
            case 0:
                set_interrupt_idt(i, (uintptr_t)&int0, 0b10001110, 0x08);
                break;
            case 32:
                set_interrupt_idt(i, (uintptr_t)&irq0, 0b10001110, 0x08);
                break;
            case 33:
                set_interrupt_idt(i, (uintptr_t)&irq1, 0b10001110, 0x08);
                break;
            case 37:
                set_interrupt_idt(i, (uintptr_t)&irq5, 0b10001110, 0x08);
                break;
            case 44:
                set_interrupt_idt(i, (uintptr_t)&irq12, 0b10001110, 0x08);
                break;
            case 0xA7:
                set_interrupt_idt(i, (uintptr_t)&syscallint, 0b11101110, 0x08);
                break;
            default:
                if (i >= 0x20 && i < 0x28) {
                    set_interrupt_idt(i, (uintptr_t)&irqmaslabel, 0b10001110, 0x08);
                } else if (i >= 0x28 && i <= 0x2F) {
                    set_interrupt_idt(i, (uintptr_t)&irqslavelabel, 0b10001110, 0x08);
                } else if (i < 0x20) {
                    set_interrupt_idt(i, (uintptr_t)&errlabel, 0b10001110, 0x08);
                } else {
                    set_interrupt_idt(i, (uintptr_t)&intlabel, 0b10001110, 0x08);
                }
                break;
        }
    }
    
    set_idt((uintptr_t)&itr);
}