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