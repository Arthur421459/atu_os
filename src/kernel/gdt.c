#include "kernel/gdt.h"
#include "lib/string.h"
extern uintptr_t stack_top;
struct gdt_entry gdt[6] __attribute__((aligned(16)));
struct gdt_ptr gp __attribute__((aligned(16)));
struct tss ktss __attribute__((aligned(16)));
#define offset (0xC0000000 - 0x100000)

void gdt_set_entry(int seg, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran) {
    gdt[seg].base_low = base & 0xFFFF;
    gdt[seg].base_middle = (base >> 16) & 0xFF;
    gdt[seg].base_high = (base >> 24) & 0xFF;

    gdt[seg].limit_low = limit & 0xFFFF;
    gdt[seg].granularity = (limit >> 16) & 0xF;
    gdt[seg].granularity |= gran & 0xF0; // 0xf0 = 111100000
    gdt[seg].access = access;
}


void config_gdt() {
    // set tss
    memset(&ktss, 0, sizeof(struct tss));
    ktss.ss0 = kerneldata_seg;
    ktss.esp0 = stack_top;
    ktss.iopb = sizeof(struct tss);

    // set gdt desc
    gp.base = (uintptr_t)&gdt;
    gp.limit = sizeof(gdt) - 1;
    // add seg
    gdt_set_entry(0,0,0,0,0); // null seg

    gdt_set_entry(1, 0, 0xFFFFFFFF, 0x9A, 0xC0); // kernel code seg 0x08
    gdt_set_entry(2, 0, 0xFFFFFFFF, 0x92, 0xC0); // kernel data seg 0x10
    
    gdt_set_entry(3, 0, 0xFFFFFFFF, 0xFA, 0xC0); // user code seg 0x18
    gdt_set_entry(4, 0, 0xFFFFFFFF, 0xF2, 0xC0); // user data seg 0x20
    
    gdt_set_entry(5, (uintptr_t)&ktss, sizeof(struct tss) - 1, 0x89, 0); // tss seg 0x28

    // load
    set_gdt((uintptr_t)&gp);
    set_tss();
}