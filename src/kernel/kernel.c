#include "lib/bool.h"
#include "lib/io.h"
#include "drivers/keyboard.h"
#include <stdint.h>
volatile char* tvideo = (volatile char*) 0xB8000;
int cursor = 0;
int cursorc = 0;

// asm functions
extern void set_gdt(uint32_t gp_ptr);
extern void int0();
extern void irq0();
extern void irq1();
extern void irqmaslabel();
extern void irqslavelabel();
extern void intlabel();
extern void set_idt(uint32_t itr);


// funções aleatorias

void set_cursor_pos(uint16_t pos) {
    outb(0x3d4, 0x0F); // set reg
    outb(0x3d5, (uint8_t) pos & 0xff); // change reg
    outb(0x3d4, 0x0e);
    outb(0x3d5, (uint8_t) ((pos >> 8) & 0xFF));
    cursorc = pos;
    cursor = pos * 2;
}
int set_cursor_pos_xy(int x, int y) {
    int abspos = y * 80 + x;
    set_cursor_pos(abspos);
    return abspos;
}

void printchar(char c, int color) {
    if (cursor >= 4000) {
        cursor = 0;
    }
    tvideo[cursor++] = c;
    tvideo[cursor++] = color;
    cursorc++;
}
void print(const char* str, int color) {
    while (*str) {
        
        printchar(*str++, color);
    }
    set_cursor_pos(cursorc);
}

void clear() {
    cursor = 0;
    while (cursor < 4000) {
        tvideo[cursor++] = 0;
        tvideo[cursor++] = 0x07;
    }
    cursor = 0;
    cursorc = 0;
}

// gdt

struct gdt_entry
{
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

struct gdt_entry gdt[5];
struct gdt_ptr gp;



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
    gp.base = (uintptr_t)&gdt;
    gp.limit = sizeof(gdt) - 1;
    gdt_set_entry(0,0,0,0,0); // null seg

    gdt_set_entry(1, 0, 0xFFFFFFFF, 0x9A, 0xC0); // kernel code seg 0x08
    gdt_set_entry(2, 0, 0xFFFFFFFF, 0x92, 0xC0); // kernel data seg 0x10
    
    gdt_set_entry(3, 0, 0xFFFFFFFF, 0xFA, 0xC0); // user code seg 0x18
    gdt_set_entry(4, 0, 0xFFFFFFFF, 0xF2, 0xC0); // user data seg 0x20
    set_gdt((uintptr_t)&gp);
}

// idt

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


struct idt_entry idt[256];
struct idt_ptr itr;
#define MasterPIC_code 0x20
#define SlavePIC_code 0xA0
#define MasterPIC_data 0x21
#define SlavePIC_data 0xA1

void int_handler(uint32_t num) {
    switch (num) {
        case 0:
            print("Voce ja estudou matematica na escola?", 0x07);
            break;
    }
}

void irq_handler(uint32_t irqx) {
    switch (irqx) {
        case 0:
            break;
        case 1:
            irq1code();
            break;
    }
    if (irqx >= 8) {
        outb(SlavePIC_code, 0x20);
    }
    outb(MasterPIC_code, 0x20);
}
void remap_pic(uint8_t master_ofs, uint8_t slave_ofs) {
    // Master = IRQ0 - IRQ7
    // Slave = IRQ8 - IRQ15

    outb(MasterPIC_code, 0x11);
    outb(SlavePIC_code, 0x11);

    outb(MasterPIC_data, master_ofs);
    outb(SlavePIC_data, slave_ofs);

    outb(MasterPIC_data, 4); // dizer para o master que o slave está ligado ao irq2 no master
    outb(SlavePIC_data, 2);

    outb(MasterPIC_data, 1); // 8080 -> 8086
    outb(SlavePIC_data, 1);

    outb(MasterPIC_data, 0); // desmascarar
    outb(SlavePIC_data, 0);
}



void set_interrupt_idt(int i,uint32_t offset, uint8_t attributes, uint16_t selector) {
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
                set_interrupt_idt(i, (uintptr_t)&int0, 0x8E, 0x08);
                break;
            case 32:
                set_interrupt_idt(i, (uintptr_t)&irq0, 0x8E, 0x08);
                break;
            case 33:
                set_interrupt_idt(i, (uintptr_t)&irq1, 0x8E, 0x08);
                break;
            default:
                if (i >= 0x20 && i < 0x28) {
                    set_interrupt_idt(i, (uintptr_t)&irqmaslabel, 0x8E, 0x08);
                } else if (i >= 0x28 && i <= 0x2F) {
                    set_interrupt_idt(i, (uintptr_t)&irqslavelabel, 0x8E, 0x08);
                } else {
                    set_interrupt_idt(i, (uintptr_t)&intlabel, 0x8E, 0x08);
                }
                break;
        }
    }
    
    set_idt((uintptr_t)&itr);

}



void read_sector(uint32_t lba, uint16_t* buffer, uint8_t sectors) {
    while ((inb(0x1F7) & 0x80)); // wait ata
    outb(0x1F2, sectors); // sectors quan

    outb(0x1F3, lba & 0xFF); // lba low
    outb(0x1F4, (lba >> 8) & 0xFF); // lba mid
    outb(0x1F5, (lba >> 16) & 0xFF); // lba high
    outb(0x1F6, 0b11100000 | ((lba >> 24) & 0x0F)); // head/drive & lba end

    outb(0x1F7, 0x20); // READDDDDDDD
    uint32_t abswords = 0;
    for (int s = 0; s < sectors; s++) {
        uint8_t status;
        do {
            status = inb(0x1F7);
            if (status & 0x01) {
                return;
            }
        } while (!(status & 0x08));
        for (int w = 0; w < 256; w++) {
            buffer[w+abswords] = inw(0x1F0);
        }
        abswords += 256;
    }
}
uint16_t buffersec[256];
uint8_t* buffersecpb = (uint8_t*)buffersec;
// ATUFS things
typedef struct {
    uint8_t name[16];
    uint8_t ext[3];
    uint16_t cluster;
    uint32_t size;
} file_entry;


extern char bss_start;
extern char bss_end;
// void clear_bss() {
//     uint8_t *p = (uint8_t*)&bss_start;
//     uint8_t *end = (uint8_t*)&bss_end;
//     while (p < end) {
//         *p++ = 0;
//     }
// }
void clear_bss() {
    uint8_t *p = (uint8_t*)&bss_start;
    for (int i = 0; i < 15; i++) {
        p[i] = 0;
    }
}

void kernel() {
    config_gdt();
    remap_pic(0x20, 0x28);
    config_idt();
    clear();
    set_cursor_pos(0);
    while (true) {
        out_key key = waitget_key();
        if (key.asciicode) {
            if (key.asciicode == '\n') {
                set_cursor_pos_xy(0, (cursorc / 80) + 1);
            } else if (key.asciicode == '\b') {
                if (cursor >= 2) {
                    cursor -= 2;
                    tvideo[cursor] = ' ';
                    tvideo[cursor + 1] = 0x07;
                    cursorc--;
                    set_cursor_pos(cursorc);
                }   
            } else {
                printchar(key.asciicode, 0x07);
                set_cursor_pos(cursorc);
            }
        }
    }
}