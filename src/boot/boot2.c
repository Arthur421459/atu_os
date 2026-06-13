#include <stdint.h>
#include "lib/io.h"
#include "lib/string.h"
#include "drivers/ata.h"
#include "lib/atufs.h"
#include "lib/elf.h"
#include "lib/mem.h"
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

volatile char* tvideo = (volatile char*) 0xB8000;
uint8_t* kernel_buffer = (uint8_t*)0x200000;

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


struct vbe_mode_info_structure vbe_info;
volatile uint32_t* graphic;
uint8_t drive = 0;
uint16_t readbufferf[256];
struct file filebuffer1;
struct file filebuffer2;
uint8_t readbuffer1[128];
uint8_t readbuffer2[64];


uintptr_t boot2main() {
    //graphic = (volatile uint32_t*) vbe_info.framebuffer;
    clear();
    read_sector(atufsinfo.file0, (uint16_t*)&filebuffer1, 1); // read file
    uint64_t rootsize = read_filedata(&filebuffer1, readbuffer1);
    find_file("kernel.elf", readbuffer1, rootsize, &filebuffer2);
    read_filedata(&filebuffer2, kernel_buffer);
    uintptr_t kernel_offset = load_elf(kernel_buffer);
    return kernel_offset;
}