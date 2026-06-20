#include "drivers/ata.h"
#include "lib/bool.h"
#include "lib/io.h"
#include "drivers/keyboard.h"
#include "lib/atufs.h"
#include "lib/string.h"
#include "drivers/cmos.h"
#include <stdint.h>
volatile char* tvideo = (volatile char*) 0xB8000;
int cursor = 0;
int cursorc = 0;

// asm functions
extern void set_gdt(uint32_t gp_ptr);
extern void int0();
extern void irq0();
extern void irq1();
extern void irq12();
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
}
int set_cursor_pos_xy(int x, int y) {
    int abspos = y * 80 + x;
    set_cursor_pos(abspos);
    return abspos;
}
void printchar_wpos(char c, int color, int pos) {
    if (pos >= 4000) {
        return;
    }
    pos *= 2;
    tvideo[pos++] = c;
    tvideo[pos++] = color;
}
void printchar_wposxy(char c, int color, int x, int y) {
    printchar_wpos(c, color, (80*y)+x);
}

void print_wpos(const char* str, int color, int pos) {
    while (*str) {
        printchar_wpos(*str++, color, pos++);
    }
}
void print_wposxy(const char* str, int color, int x, int y) {
    print_wpos(str, color, (80*y)+x);
}

void clear() {
    for (short i = 0; i < 4000;i++) {
        if (i % 2) {
            tvideo[i] = 0x07;
        } else {
            tvideo[i] = 0;
        }
    }
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
            print_wpos("Voce ja estudou matematica na escola?", 0x07, 0);
            break;
    }
}
int a = 0;
int tick = 0;
uint32_t systime = 0;
nixt worldtime = 0;
void irq_handler(uint32_t irqx) {
    switch (irqx) {
        case 0:
            tick++;
            if (tick >= 100) {
                systime++;
                worldtime++;
                tick = 0;
            }
            break;
        case 1:
            irq1code();
            break;
        case 12:
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
            case 44:
                set_interrupt_idt(i, (uintptr_t)&irq12, 0x8E, 0x08);
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

extern char bss_start;
extern char bss_end;
void clear_bss() {
    uint8_t *p = (uint8_t*)&bss_start;
    for (int i = 0; i < bss_end; i++) {
        p[i] = 0;
    }
}

void sleep(uint32_t sec) {
    uint32_t target = systime + sec;
    while (systime < target) {
        asm volatile ("hlt");
    }
}

struct file filebuffer1;
struct file filebuffer2;

uint16_t buffer1[256];
uint16_t buffer2[1024];
uint16_t buffer3[256];

uint16_t start_pos;

uint8_t cmd_size;
char cmd_buffer[128];
uint8_t cmd_pos;

uint64_t rootsize = 45;
char numbuffer[5];

char teste[] = "In computing, a file system or filesystem (often abbreviated to FS or fs) governs file organization and access. A local file system is a capability of an operating system that services the applications running on the same computer.[1][2] A distributed file system is a protocol that provides file access between networked computers. A file system provides a data storage service that allows applications to share mass storage. Without a file system, applications could access the storage in incompatible ways that lead to resource contention, data corruption, and data loss. There are many file system designs and implementations – with various structures and features and various resulting characteristics such as speed, flexibility, security, size, and more. File systems have been developed for many types of storage devices, including hard disk drives (HDDs), solid-state drives (SSDs), magnetic tapes and optical discs.[3] A portion of the computer main memory can be set up as a RAM disk that serves as a storage device for a file system. File systems such as tmpfs can store files in virtual memory. A virtual file system provides access to files that are either computed on request, called virtual files (for example those provided by procfs and sysfs), or are mapping into another, backing storage. \0\0\0";

void cmd_end() {
    uint16_t nextpos = start_pos+cmd_size - ((start_pos+cmd_size) % 80) + 80;
    if (cmpstr("echo ", cmd_buffer)) {
        print_wposxy(cmd_buffer+5, 0x07, 0, (start_pos+cmd_size)/80 + 1);
        nextpos += 160;
    } else if (cmpstr("cat ", cmd_buffer)) {
        find_file(cmd_buffer+4, (uint8_t*)buffer1, 45, &filebuffer2);
        uint64_t size = read_filedata(&filebuffer2, (uint8_t*)buffer2);
        buffer2[size] = '\0';
        print_wposxy((char*)buffer2, 0x07, 0, start_pos/80 + 1);
        nextpos += 80 + 80*(((uint32_t)size + 79) / 80);
    } else if (cmpstr("time", cmd_buffer)) {
        print_wposxy("Unix Time: ", 0x07, 0, start_pos/80 + 1);
        num_to_str(worldtime, numbuffer);
        print_wposxy(numbuffer, 0x07, 11, start_pos/80 + 1);
        nextpos += 80;
    } else if (cmpstr("edit ", cmd_buffer)) {
        uint32_t file = find_file(cmd_buffer+5, (uint8_t*)buffer1, 45, &filebuffer2);
        if (file) {
            write_file(&filebuffer2, file, (uint8_t*)teste, 1310);
        }
    }
    // final

    print_wpos("atuos>", 0x07, nextpos);
    set_cursor_pos(nextpos + 7);
    memset(cmd_buffer, 0, 128);
    start_pos = nextpos+7;
    cmd_size = 0;
    cmd_pos = 0;

}
#define initialpic_freq 1193182
void set_pit_freq(uint32_t freq) {
    uint16_t div = initialpic_freq / freq;
    outb(0x43, 0b00110111); // set command (channel 0, square wave)

    outb(0x40, div & 0xFF); // low
    outb(0x40, div >> 8); // high
}

void kernel() {
    config_gdt();
    remap_pic(0x20, 0x28);
    set_pit_freq(100);
    config_idt();
    worldtime = convert_to_nixt(get_cmos_time());
    worldtime -= 10800; // sincronizar fuso horário
    init_atufs();
    clear();

    print_wposxy("Bem vindo ao AtuOS! :D", 0x07, 0, 1);
    set_cursor_pos_xy(7, 3);
    print_wposxy("atuos>", 0x07, 0, 3);
    start_pos = 247;
    read_sector(atufsinfo.file0, (uint16_t*)&filebuffer1, 1);
    rootsize = read_filedata(&filebuffer1, (uint8_t*)buffer1);
    // start cmd
    while (true) {
        out_key key = waitget_key();
        switch (key.asciicode) {
            case '\b':
                if (cmd_pos < 1) {break;}
                cmd_pos--;
                cmd_size--;
                printchar_wpos(' ', 0x07, start_pos+cmd_pos);
                cmd_buffer[cmd_pos] = ' ';
                set_cursor_pos(start_pos+cmd_pos);
                break;
            case '\n':
                cmd_end();
                break;
            case 0:
                break;
            default:
                if (cmd_pos >= 128) {break;}
                printchar_wpos(key.asciicode, 0x07, start_pos+cmd_pos);
                cmd_buffer[cmd_pos++] = key.asciicode;
                cmd_size++;
                set_cursor_pos(start_pos+cmd_pos);
        }
    }
}