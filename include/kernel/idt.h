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
extern void int1();
extern void int2();
extern void int3();
extern void int4();
extern void int5();
extern void int6();
extern void int7();
extern void int8();
extern void int10();
extern void int11();
extern void int12();
extern void int13();
extern void int14();
extern void int16();
extern void int17();
extern void int18();
extern void int19();
extern void int20();
extern void int21();

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

struct int_stack {
    uint32_t eax;
    uint32_t ebx;
    uint32_t ecx;
    uint32_t edx;

    void* edi;
    void* esi;
    uint32_t ebp;

    uint32_t ds;
    uint32_t es;
    uint32_t fs;
    uint32_t gs;
    
    uint32_t num;
    uint32_t err;
    
    void* eip;
    uint32_t cs;
    uint32_t eflags;
} __attribute__((aligned));

extern struct idt_entry idt[256];
extern struct idt_ptr itr;
void set_interrupt_idt(int i, uint32_t offset, uint8_t attributes, uint16_t selector) ;
void config_idt();

#endif