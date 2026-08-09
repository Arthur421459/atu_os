#include "lib/main.h"
#include "kernel/bootinfo.h"
#include "lib/io.h"
#include "drivers/ata.h"
#include "lib/atufs.h"
#include "lib/elf.h"
#include "lib/string.h"
volatile char* tvideo = (volatile char*) 0xB8000;
uint8_t* kernel_buffer = (uint8_t*)0x500000;

int cursor = 0;
int cursorc = 0;
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

void printchar(char c, uint8_t color) {
    if (cursor >= 4000) {
        cursor = 0;
    }
    tvideo[cursor++] = c;
    tvideo[cursor++] = color;
    cursorc++;
}
void print(const char* str, uint8_t color) {
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

struct mbr_partition {
    uint8_t bootable;
    uint8_t start_chs[3];
    uint8_t type;
    uint8_t end_chs[3];
    uint32_t start_lba;
    uint32_t sectors;
};

struct vbe_mode_info_structure vbe_info;
uint8_t drive = 0;
uint8_t* partaddr;
uint32_t total_smaps;
struct boot_info* bootinfo = (struct boot_info*)0x5000;
uint16_t readbufferf[256];
struct file filebuffer1;
struct file filebuffer2;
uint8_t readbuffer1[1024];
uint8_t readbuffer2[64];
uintptr_t load_kernel(uint8_t* buffer) {
    if (!(is_compatible(buffer))) {return 0;}
    struct elf_header* elfh = (struct elf_header*)buffer;
    struct ph_entry* ph_entries = (struct ph_entry*)(buffer+elfh->pheader_ofs);
    for (int i = 0; i < elfh->entrynum_ph; i++) {
        struct ph_entry entry = ph_entries[i];
        if (entry.seg_type != 1) {continue;} // se não for PT_LOAD então pule
        memcpy((void*)entry.p_paddr, buffer+entry.p_offset, entry.p_filesz); // copiar para o endereço correto
        memset((void*)(entry.p_paddr+entry.p_filesz), 0, entry.p_memsz - entry.p_filesz); // preencher o bss
    }
    return elfh->pentry_ofs; // return offset
}
uintptr_t boot2main() {
    set_partstart(partaddr);
    init_atufs();
    read_sector_part(atufsinfo.file0, (uint16_t*)&filebuffer1, 1); // read file
    uint64_t rootsize = read_filedata(&filebuffer1, readbuffer1);
    find_file("kernel.elf", readbuffer1, rootsize, &filebuffer2);
    read_filedata(&filebuffer2, kernel_buffer);
    uintptr_t kernel_offset = load_kernel(kernel_buffer);
    // struct elf_header* elfh = (struct elf_header*)kernel_buffer;

    // uintptr_t bytes_counter = 0;
    // struct ph_entry* ph_entries = (struct ph_entry*)(kernel_buffer+elfh->pheader_ofs);
    // for (int i = 0; i < elfh->entrynum_ph;i++) {
    //     struct ph_entry entry = ph_entries[i];
    //     if (entry.seg_type != 1) {continue;}
    //     memcpy((void*)(0x100000+bytes_counter), kernel_buffer+entry.p_offset, entry.p_filesz);
    //     memset((void*)(entry.p_vaddr+entry.p_filesz), 0, entry.p_memsz - entry.p_filesz);
    //     bytes_counter += entry.p_memsz;
    // }

    bootinfo->drive = drive;
    bootinfo->partaddr = partaddr;
    bootinfo->vbe_info = &vbe_info;
    bootinfo->smaps = (struct smap*)0x500;
    bootinfo->total_smaps = total_smaps;
    if (!kernel_offset) {
        asm volatile ("int $0");
    }
    memset((void*)0x500000, 0, 1 << 20);
    return kernel_offset;
}