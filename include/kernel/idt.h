#ifndef IDT_H
#define IDT_H
#include "lib/main.h"
extern void set_idt(uint32_t itr);
struct idt_entry {
    uint16_t low_offset;
    uint16_t selector;
    uint8_t reserved_zero;
    uint8_t attributes;
    uint16_t high_offset;
} __attribute__((packed));
struct idt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));
// exceptions
extern void int0();

// irqs
extern void irq0();
extern void irq1();
extern void irq5();
extern void irq12();

// label
extern void irqmaslabel();
extern void irqslavelabel();
extern void intlabel();
extern void errlabel();

// syscall
extern void syscallint();

extern struct idt_entry idt[256];
extern struct idt_ptr itr;
void set_interrupt_idt(int i, uint32_t offset, uint8_t attributes, uint16_t selector) ;
void config_idt();

#endif