#include "drivers/ata.h"
#include "lib/main.h"
#include "kernel/bootinfo.h"
#include "lib/io.h"
#include "drivers/keyboard.h"
#include "lib/atufs.h"
#include "lib/string.h"
#include "drivers/cmos.h"
#include "lib/elf.h"
#include "kernel/paging.h"
#include "kernel/gdt.h"
#include "kernel/apic.h"
#include "kernel/heap.h"



volatile char* tvideo = (volatile char*) 0xB8000;
extern uintptr_t stack_top;

int cursor = 0;
int cursorc = 0;

// asm functions
extern void set_pag(uintptr_t addr);
extern void jmp_prog(uintptr_t eip, uintptr_t esp, uintptr_t cr3) __attribute__((noreturn));

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
// idt
void int_handler(uint32_t num) {
    switch (num) {
        case 0:
            print_wpos("Voce ja estudou matematica na escola?", 0x07, 0);
            break;
    }
    ktss.esp0 = stack_top;
    ktss.ss0 = kerneldata_seg;
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
    ktss.esp0 = stack_top;
    ktss.ss0 = kerneldata_seg;
}

void sleep(uint32_t sec) {
    uint32_t target = systime + sec;
    
    while (systime < target) {
        asm volatile ("hlt");
    }
}

void usleep(uint32_t usec) {
    uint32_t ticks_to_wait = (usec + 9) / 10;
    
    uint32_t start_total_ticks = (systime * 100) + tick;
    uint32_t target_total_ticks = start_total_ticks + ticks_to_wait;
    while (((systime * 100) + tick) < target_total_ticks) {
        asm volatile ("hlt");
    }
}

uint16_t start_pos;

uint8_t cmd_size;
char cmd_buffer[128];
uint8_t cmd_pos;

uint64_t rootsize = 45;
char numbuffer[5];

extern struct boot_info* binfo;


struct irqotherstack {
    uint32_t sysenterorint;
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

void add_page_essential(uint32_t* ptr, uint32_t physptrpage) {
    memcpy(ptr, page_directory, 4096); // get the base
    ptr[832] = 0;
    map_page(ptr, physptrpage, 0xD0000, 1, page_present | page_writable | sysmisc_pagedir,
    page_present | page_writable | sysmisc_pagetble | full_pagedir);
}


struct load_program_result {
    uint32_t pentry;
    uint32_t pagediraddr;
    uint32_t stackaddr;
};
struct load_program_result load_program(uint8_t* programptr) {
    struct load_program_result prog = {0};
    if (!is_compatible(programptr)) return prog;
    uint32_t pagedirpage = ppalloc(1);
    uint32_t* pagediraddr = phys_to_virt(pagedirpage, 1, sysmisc_pagetble | page_present);
    add_page_essential(pagediraddr, pagedirpage);
    struct elf_header* elfh = (struct elf_header*)programptr;
    struct ph_entry* ph_entries = (struct ph_entry*)(programptr+elfh->pheader_ofs);
    for (int i = 0; i < elfh->entrynum_ph; i++) {
        struct ph_entry entry = ph_entries[i];
        if (entry.seg_type != 1) continue;
        uint8_t* ph_ptr = (uint8_t*)amalloc(entry.p_memsz, page_present | page_writable | sysmisc_pagetble,
        page_present | page_writable | sysmisc_pagedir);
        struct malloc_header* hd = (struct malloc_header*)(ph_ptr-sizeof(struct malloc_header));
        memcpy(ph_ptr, programptr+entry.p_offset, entry.p_filesz);
        map_page(pagediraddr, hd->phys_page, entry.p_vaddr >> 12, (entry.p_memsz+4095) >> 12,
        page_present | page_writable | user_page | prog_pagetble, 
        page_present | page_writable | user_page | prog_pagedir);
    }
    uint32_t progstackpage = ppalloc(8);
    map_page(pagediraddr, progstackpage, 0xB0000, 8, page_present | page_writable | progstack_pagetble | user_page, page_present | page_writable | progstack_pagedir | user_page);
    prog.pentry = elfh->pentry_ofs;
    prog.pagediraddr = pagedirpage << 12;
    prog.stackaddr = 0xB0007ffc;
    free_directmap(pagediraddr, 1);
    return prog;
}

struct file* file0;
uint8_t* file0data;
uintptr_t file0size;
void kernel() {
    worldtime = convert_to_nixt(get_cmos_time());
    tvideo = phys_to_virt(0xb8, 1, page_present | page_writable);
    clear();

    file0 = malloc(512, page_present | page_writable, page_present | page_writable);
    read_sector_part(atufsinfo.file0, (uint16_t*)file0, 1);
    file0data = malloc(file0->size_low, page_present | page_writable, page_present | page_writable);
    file0size = read_filedata(file0, file0data);

    struct file* cmdfile = malloc(512, page_present | page_writable, page_present | page_writable);
    find_file("test", file0data, file0size, cmdfile);

    uint8_t* cmddata = malloc(cmdfile->size_low, page_present | page_writable, page_present | page_writable);
    read_filedata(cmdfile, cmddata);
    free(cmdfile);
    print_wpos("Hello! :DDD", 0x07, 0);
    struct load_program_result loadedcmd = load_program(cmddata);
    jmp_prog(loadedcmd.pentry, loadedcmd.stackaddr, loadedcmd.pagediraddr);
    while(1);
}




struct syscall_result {
    uint32_t ret;
    uint32_t eax;
    uint32_t ebx;
    uint32_t ecx;
    uint32_t edx;
    void* esi;
    void* edi;
};

struct syscall_result syscall_c(uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx, void* esi, void* edi) {
    struct syscall_result result;
    result.eax = eax;
    result.ebx = ebx;
    result.ecx = ecx;
    result.edx = edx;
    result.esi = esi;
    result.edi = edi;
    switch (eax) {
        case 0:
            while(1);
            break;
        case 1:
            // esi: pointer, ebx: pos
            print_wpos((char*)esi, 0x07, ebx);
            break;
        case 2:
            // ebx = seconds
            asm volatile ("sti");
            sleep(ebx);
            asm volatile ("cli");
            break;
        case 3: {
            uint32_t old;
            old = irqcount[ebx];
            asm volatile("sti");

            while (irqcount[ebx] == old) {
                asm volatile("hlt");
            }
            asm volatile("cli");
            break; }
        case 4:
            // ebx bytes =edi pointer
            result.edi = malloc(ebx, page_present | page_writable | user_page | progalloc_pagetble, page_present | page_writable | user_page | progalloc_pagedir);
            break;
        case 5:
            free(edi);
            break;
        case 0x82:
            // ebx = microseconds
            asm volatile ("sti");
            usleep(ebx);
            asm volatile ("cli");
            break;
        case 10: {
            // edi filebuffer =eax filenum    ebx root, esi name
            uint64_t size;
            struct file* rootbuffer = malloc(sizeof(struct file), page_present | page_writable, page_present | page_writable);
            read_sector_part(atufsinfo.file0+ebx, (uint16_t*)&rootbuffer, 1);
            uint16_t* buffer = malloc(rootbuffer->size_low, page_present | page_writable, page_present | page_writable);
            size = read_filedata(rootbuffer, (uint8_t*)buffer);
            result.eax = find_file(esi, (uint8_t*)buffer, size, edi);
            free(rootbuffer);
            free(buffer);
            break; }
        case 11: {
            // edi buffer =eax sizel =ebx sizeh     esi filebuffer
            uint64_t size;
            size = read_filedata((struct file*)esi, edi);
            result.eax = 0;
            result.ebx = 0;
            result.eax = (uint32_t)size;
            result.ebx = size >> 32 & 0xFFFF;
            break;
        }
    }
    result.ret = 0;
    return result;
}
struct syscallenteruserstack {
    uint32_t ecx;
    uint32_t edx;
} __attribute__((packed));
struct syscallenterstack {
    uint32_t eax;
    uint32_t ebx;

    void* edi;
    void* esi;
    uint32_t ebp;

    uint32_t ds;
    uint32_t es;
    uint32_t fs;
    uint32_t gs;
    
    void* esp;
    void* eip;

} __attribute__((packed));
uintptr_t syscall_enter(struct syscallenterstack* stack) {
    struct syscallenteruserstack* u = stack->esp;
    struct syscall_result a = syscall_c(stack->eax, stack->ebx, u->ecx, u->edx, stack->esi, stack->edi);
    stack->eax = a.eax;
    stack->ebx = a.ebx;
    u->ecx = a.ecx;
    u->edx = a.edx;

    stack->esi = a.esi;
    stack->edi = a.edi;

    ktss.esp0 = stack_top;
    ktss.ss0 = kerneldata_seg;
    return a.ret;
}

struct syscallintstack {
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

uintptr_t syscall_int(struct syscallintstack* stack) {
    ktss.esp0 = stack_top;
    ktss.ss0 = kerneldata_seg;
    struct syscall_result a = syscall_c(stack->eax, stack->ebx, stack->ecx, stack->edx, stack->esi, stack->edi);
    stack->eax = a.eax;
    stack->ebx = a.ebx;
    stack->ecx = a.ecx;
    stack->edx = a.edx;

    stack->esi = a.esi;
    stack->edi = a.edi;

    ktss.esp0 = stack_top;
    ktss.ss0 = kerneldata_seg;
    return a.ret;
}