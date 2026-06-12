#include <stdint.h>
#include "lib/io.h"
#include "lib/string.h"
#include "drivers/ata.h"
struct vbe_mode_info_structure {
	uint16_t attributes;		// deprecated, only bit 7 should be of interest to you, and it indicates the mode supports a linear frame buffer.
	uint8_t window_a;			// deprecated
	uint8_t window_b;			// deprecated
	uint16_t granularity;		// deprecated; used while calculating bank numbers
	uint16_t window_size;
	uint16_t segment_a;
	uint16_t segment_b;
	uint32_t win_func_ptr;		// deprecated; used to switch banks from protected mode without returning to real mode
	uint16_t pitch;			// number of bytes per horizontal line
	uint16_t width;			// width in pixels
	uint16_t height;			// height in pixels
	uint8_t w_char;			// unused...
	uint8_t y_char;			// ...
	uint8_t planes;
	uint8_t bpp;			// bits per pixel in this mode
	uint8_t banks;			// deprecated; total number of banks in this mode
	uint8_t memory_model;
	uint8_t bank_size;		// deprecated; size of a bank, almost always 64 KB but may be 16 KB...
	uint8_t image_pages;
	uint8_t reserved0;

	uint8_t red_mask;
	uint8_t red_position;
	uint8_t green_mask;
	uint8_t green_position;
	uint8_t blue_mask;
	uint8_t blue_position;
	uint8_t reserved_mask;
	uint8_t reserved_position;
	uint8_t direct_color_attributes;

	uint32_t framebuffer;		// physical address of the linear frame buffer; write here to draw to the screen
	uint32_t off_screen_mem_off;
	uint16_t off_screen_mem_size;	// size of memory in the framebuffer but not being displayed on the screen
	uint8_t reserved1[206];
} __attribute__ ((packed));
struct atufs_info {
    uint16_t jmpormagic;
    uint16_t magic;
    uint8_t ver;
    uint16_t block_size;
    uint16_t file_size;
    uint16_t boot2_size;
    uint8_t skipped_blocks;
    uint32_t files;
    uint32_t clusters;
    uint32_t alocated_files;
    uint32_t alocated_clusters;
    uint16_t journal_size;
    uint32_t startbmpfile;
    uint32_t startbmpcluster;
    uint32_t file0;
    uint32_t cluster0;
    uint8_t label[8];
    uint8_t zero[460];
} __attribute__((packed));
struct extent {
    uint32_t startcluster;
    uint32_t manyclusters;
};
struct file {
    uint32_t size_low;
    uint16_t size_high;
    uint32_t last_mod;
    uint32_t last_access;
    uint32_t creation;
    uint16_t user_id;
    uint8_t attributes;
    uint8_t future;
    union {
        uint8_t data[490];
        struct extent extents[61];
    } __attribute__((packed));
} __attribute__((packed));
struct entry {
    uint32_t file;
    uint16_t entry_size;
    uint8_t atr;
    uint8_t namesize;
    uint8_t name[];
};
volatile char* tvideo = (volatile char*) 0xB8000;
uint8_t* kernel_addr = (uint8_t*)0x100000;
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



struct atufs_info atufsinfo;
struct vbe_mode_info_structure vbe_info;
volatile uint32_t* graphic;
uint8_t drive = 0;
uint16_t readbufferf[256];
struct file filebuffer1;
struct file filebuffer2;
uint8_t readbuffer1[128];
uint8_t readbuffer2[64];
uint64_t read_filedata(struct file* file1, uint8_t* buffer) {
    if (file1->attributes & 0b10000000) {
        uint64_t size;
        size = ((uint64_t) file1->size_high << 32) | file1->size_low;
        return size;
    } else {
        for (uint32_t i = 0; i < file1->size_low;i++) {
            buffer[i] = file1->data[i];
        }
        return (uint64_t)file1->size_low;
    }
    return 0;
}
void find_file(const char* name, uint8_t* buffer, uint64_t buffer_size, struct file* f) {
    uint64_t offset = 0;
    while (offset < buffer_size) {
        struct entry* fentry = (struct entry*)(buffer+offset);
        if (cmpstr_limit((char*)fentry->name, name, fentry->namesize)) {
            read_sector(atufsinfo.file0+(fentry->file), (uint16_t*)f, 1);
            return;
        }
        offset += fentry->entry_size;
    };
}
void boot2main() {
    //graphic = (volatile uint32_t*) vbe_info.framebuffer;
    clear();
    read_sector(0, (uint16_t*)&atufsinfo, 1); // update atufsinfo
    read_sector(atufsinfo.file0, (uint16_t*)&filebuffer1, 1); // read file
    uint64_t rootsize = read_filedata(&filebuffer1, readbuffer1);
    find_file("teste.txt", readbuffer1, rootsize, &filebuffer2);
    uint64_t filesize = read_filedata(&filebuffer2, readbuffer2);
    for (uint64_t i = 0; i < filesize;i++) {
        printchar(readbuffer2[i], 0x07);
    }
}