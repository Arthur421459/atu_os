#include "drivers/ata.h"
#include "lib/io.h"
#include <stdint.h>
void read_sector(uint32_t lba, uint16_t* buffer, uint8_t sectors) {
    while ((inb(0x1F7) & 0x80)); // wait ata
    outb(0x1F2, sectors); // sectors quan

    outb(0x1F3, lba & 0xFF); // lba low
    outb(0x1F4, (lba >> 8) & 0xFF); // lba mid
    outb(0x1F5, (lba >> 16) & 0xFF); // lba high
    outb(0x1F6, 0b11100000 | ((lba >> 24) & 0x0F)); // head/drive & lba end

    outb(0x1F7, 0x20); // READDDDDDDD
    uint16_t sc = sectors;
    if (sc == 0) {
        sc = 256;
    }
    for (int s = 0; s < sc; s++) {
        uint8_t status;
        do {
            status = inb(0x1F7);
            if (status & 1) {
                return;
            }
        } while (!(status & 8));
        for (int j = 0; j < 256; j++) {
            *buffer++ = inw(0x1F0);
        }
    }
}
void write_sector(uint32_t lba, uint16_t *buffer, uint8_t sectors) {
    while ((inb(0x1F7) & 0x80)); // wait ata
    outb(0x1F2, sectors); // sectors quan

    // lba
    outb(0x1F3, lba & 0xFF); // low
    outb(0x1F4, (lba >> 8) & 0xFF); // medium
    outb(0x1F5, (lba >> 16) & 0xFF); // high
    // lba end
    
    outb(0x1F6, 0b11100000 | ((lba >> 24) & 0x0F));
    outb(0x1F7, 0x30); // WRITEEEEEEE

    for (uint8_t i = 0; i < sectors; i++) {
        uint8_t status;
        do {
            status = inb(0x1F7);
            if (status & 1) {return;}
        } while (!(status & 8));
        
        for (uint16_t j = 0; j < 256;j++) {
            outw(0x1F0, *buffer++);
        }
    }
    outb(0x1F7, 0xE7);
    while ((inb(0x1F7) & 0x80));
}
void lread_sector(uint32_t lba, uint16_t *buffer, uint32_t sectors) {
    for (uint32_t i = 0; i < (sectors / 256);i++) {
        read_sector(lba, buffer, 0);
        buffer += 65536; // 256^2
        lba += 256;
    }
    if (sectors % 256) {
        read_sector(lba, buffer, sectors % 256);
        lba += sectors % 256;
        buffer += (sectors % 256)*256;
    }
}
void lwrite_sector(uint32_t lba, uint16_t *buffer, uint32_t sectors) {
    for (uint32_t i = 0; i < (sectors / 256);i++) {
        write_sector(lba, buffer, 0);
        buffer += 65536; // 256 * words per sector
        lba += 256;
    }
    if (sectors % 256) {
        write_sector(lba, buffer, sectors % 256);
        lba += sectors % 256;
        buffer += (sectors % 256)*256;
    }
}