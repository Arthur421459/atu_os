#include "drivers/ata.h"
#include "lib/bool.h"
#include "lib/bootinfo.h"
#include "lib/io.h"
#include "drivers/keyboard.h"
#include "lib/atufs.h"
#include "lib/string.h"
#include "drivers/cmos.h"
#include "lib/elf.h"
#include <stdint.h>
volatile char* tvideo = (volatile char*) 0xB8000;
int cursor = 0;
int cursorc = 0;

// asm functions
extern void set_gdt(uint32_t gp_ptr);
extern void int0();
extern void irq0();
extern void irq1();
extern void irq5();
extern void irq12();
extern void irqmaslabel();
extern void irqslavelabel();
extern void intlabel();
extern void set_idt(uint32_t itr);
extern void syscallasm();

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
volatile uint32_t tick = 0;
volatile uint32_t systime = 0;
volatile nixt worldtime = 0;
volatile uint32_t irqcount[16];
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
        case 5:
            inb(0x22E);
            break;
        case 12:
            break;
    }
    irqcount[irqx]++;
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
            case 37:
                set_interrupt_idt(i, (uintptr_t)&irq5, 0x8E, 0x08);
                break;
            case 44:
                set_interrupt_idt(i, (uintptr_t)&irq12, 0x8E, 0x08);
                break;
            case 0xA7:
                set_interrupt_idt(i, (uintptr_t)&syscallasm, 0x8E, 0x08);
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

void msecsleep(uint32_t msec) {
    uint32_t ticks_to_wait = (msec + 9) / 10;
    
    uint32_t start_total_ticks = (systime * 100) + tick;
    uint32_t target_total_ticks = start_total_ticks + ticks_to_wait;
    while (((systime * 100) + tick) < target_total_ticks) {
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

uint8_t* programelf = (uint8_t*)0x500000;
uint32_t raminbytes = 0;
uintptr_t start;
char *argv[20];

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
    } else if (cmpstr("run ", cmd_buffer)) {
        argv[0] = cmd_buffer;
        int a = 1;
        for (int i = 0;i < cmd_size;i++) {
            if (cmd_buffer[i] == 0x20) {
                cmd_buffer[i] = 0;
                argv[a++] = cmd_buffer+i+1;
            }
        }
        find_file(argv[1], (uint8_t*)buffer1, rootsize, &filebuffer2);
        read_filedata(&filebuffer2, programelf);
        uintptr_t programoffset = load_elf(programelf);
        if (!programoffset) {
            programoffset = start;
        }
        print_wpos("atuos>", 0x07, nextpos);
        set_cursor_pos(nextpos + 7);
        start_pos = nextpos+7;
        cmd_size = 0;
        cmd_pos = 0;

        int argc = a;

        asm volatile(
            "pushl %1\n "
            "pushl %2\n "
            "pushl %3\n "
            "jmp *%0\n  "
            :
            : "r" (programoffset), "r" (argv), "r" (argc), "r" (start)
            : "memory"
        );
    } else if (cmpstr("ram", cmd_buffer)) {
        num_to_str(raminbytes >> 20, numbuffer);
        print_wposxy("RAM in MB: ", 0x07, 0, start_pos/80 + 1);
        print_wposxy(numbuffer, 0x07, 11, start_pos/80 + 1);
        nextpos += 80;
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

struct boot_info* binfo;
void calculateram() {
    for (int i = 0; i < binfo->total_smaps;i++) {
        
        if (binfo->smaps[i].type == 1) {
            raminbytes += binfo->smaps[i].length;
        }
    }
}
struct syscallstack {

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
    
    void* eip;
    uint32_t cs;
    uint32_t eflags;
} __attribute__((packed));

void set_irq5() {
    outb(0x224, 0x80);
    outb(0x225, 2);
}

void kernel() {
    config_gdt();
    remap_pic(0x20, 0x28);
    set_pit_freq(100);
    config_idt();
    worldtime = convert_to_nixt(get_cmos_time());
    set_partstart(binfo->partaddr);
    init_atufs();
    clear();
    set_irq5();
    memset((void*)0x500000, 0, 1 << 20);
    calculateram();
    print_wposxy("Bem vindo ao AtuOS! :D", 0x07, 0, 1);
    set_cursor_pos_xy(7, 3);
    print_wposxy("atuos>", 0x07, 0, 3);
    start_pos = 247;
    read_sector_part(atufsinfo.file0, (uint16_t*)&filebuffer1, 1);
    rootsize = read_filedata(&filebuffer1, (uint8_t*)buffer1);
    // start cmd
    start = (uintptr_t)&&inicio;
    inicio:
    memset(cmd_buffer, 0, 128);
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
                rootsize = read_filedata(&filebuffer1, (uint8_t*)buffer1);
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
uint32_t old;
uint64_t size;
uintptr_t syscall_c(struct syscallstack* stack) {
    switch (stack->eax) {
        case 0:
            stack->eip = (void*)start;
            break;
        case 1:
            // esi: pointer, ebx: pos
            print_wpos((char*)stack->esi, 0x07, stack->ebx);
            break;
        case 2:
            // ebx = seconds
            asm volatile ("sti");
            sleep(stack->ebx);
            asm volatile ("cli");
            break;
        case 3:
            old = irqcount[stack->ebx];

            asm volatile("sti");

            while (irqcount[stack->ebx] == old) {
                asm volatile("hlt");
            }

            asm volatile("cli");
            break;
        case 0x82:
            // ebx = microseconds
            asm volatile ("sti");
            msecsleep(stack->ebx);
            asm volatile ("cli");
            break;
        case 10:
            // edi filebuffer =eax filenum    ebx root, esi name
            read_sector_part(atufsinfo.file0+stack->ebx, (uint16_t*)&filebuffer2, 1);
            size = read_filedata(&filebuffer2, (uint8_t*)buffer2);
            stack->eax = find_file(stack->esi, (uint8_t*)buffer2, size, stack->edi);
            break;
        case 11:
            // edi buffer =eax sizel =ebx sizeh     esi filebuffer
            size = read_filedata((struct file*)stack->esi, stack->edi);
            stack->eax = (uint32_t)size;
            stack->ebx = size >> 32 & 0xFFFF;
            break;
    }
    return 0;
}