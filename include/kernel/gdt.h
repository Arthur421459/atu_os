#ifndef GDT_H
#define GDT_H
#include "lib/main.h"
#define kernelcode_seg 0x08
#define kerneldata_seg 0x10

#define usercode_seg 0x18
#define userdata_seg 0x20

#define tss_seg      0x28
extern void set_gdt(uintptr_t gp_ptr);
extern void set_tss();

struct gdt_entry {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t  base_middle;
    uint8_t  access;
    uint8_t  granularity;
    uint8_t  base_high;
} __attribute__((packed));

struct gdt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));



struct tss {
    uint16_t prev;
    uint16_t r0;
    uint32_t esp0;

    uint16_t ss0;
    uint16_t r1;
    uint32_t esp1;

    uint16_t ss1;
    uint16_t r2;
    uint32_t esp2;

    uint16_t ss2;
    uint16_t r3;

    uint32_t cr3;
    uint32_t eip;
    uint32_t eflags;

    uint32_t eax;
    uint32_t ecx;
    uint32_t edx;
    uint32_t ebx;

    uint32_t esp;
    uint32_t ebp;

    uint32_t esi;
    uint32_t edi;

    uint32_t es;
    uint32_t cs;
    uint32_t ss;
    uint32_t ds;
    uint32_t fs;
    uint32_t gs;

    uint16_t ldtr;
    uint16_t r4;

    uint16_t r5;
    uint16_t iopb;

    uint32_t ssp;
} __attribute__((packed));

extern struct gdt_entry gdt[6];
extern struct gdt_ptr gp;
extern struct tss ktss;

void gdt_set_entry(int seg, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran);
void config_gdt();

#endif