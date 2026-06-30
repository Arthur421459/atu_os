#include <stdint.h>
#include "atuos/time.h"
#include "lib/bool.h"
#include "lib/io.h"
#include "atuos/core.h"
#include "atuos/file.h"
int cursor = 0;
struct file filebuffer1;
uint8_t* buffer1 = (uint8_t*)0x400000;
uint32_t music_sizel;
uint32_t music_sizeh;
uint8_t* a = (uint8_t*)0xB8000;
bool reset_dsp() {
    outb(0x226, 1);
    usleep(10);
    outb(0x226, 0);
    while (!(inb(0x22E) & 0x80));
    return (inb(0x22A) == 0xAA);
}
void play_pcm(uint32_t srate, uint8_t bits, uint8_t* buffer, uint16_t size) {
    uint32_t bufferaddr = (uint32_t)buffer;
    if (!reset_dsp()) return;
    outb(0x22C, 0xD1); // enable speaker
    if (bits == 8) {
        outb(0x0A, 5); // disable channel 1
        outb(0x0C, 0);
        
        outb(0x0B, 0x49); // mode + channel

        outb(0x83, (bufferaddr >> 16) & 0xFF); // page
        outb(0x02, bufferaddr & 0xFF); // pos low
        outb(0x02, (bufferaddr >> 8) & 0xFF); // pos high

        outb(0x03, size & 0xFF); // length low
        outb(0x03, (size >> 8) & 0xFF); // length high

        outb(0x0A, 1); // enable channel 1
    } else if (bits == 16) {

    }
    //uint8_t time_constant = 65536 - (256000000 / (channels * srate));
    while (inb(0x22C) & 0x80); // wait for write
    outb(0x22C, 0x41);
    while (inb(0x22C) & 0x80); // wait for write
    outb(0x22C, srate >> 8);
    outb(0x22C, srate & 0xFF);
    if (bits == 8) {
        while (inb(0x22C) & 0x80);
        outb(0x22C, 0xC0); // 8 bit
        while (inb(0x22C) & 0x80);
        outb(0x22C, 0); // mono (uma saída de som apenas) sem sinal (apenas numeros naturais)
    } else if (bits == 16) {

    }
    size--;
    while (inb(0x22C) & 0x80);
    outb(0x22C, size & 0xFF);
    while (inb(0x22C) & 0x80);
    outb(0x22C, size >> 8);
}
#define bit16max 0xFFFF


volatile uint8_t* tvideo = (volatile uint8_t*)0xb8000;
void main(int argc, char *argv[]) {
    if (argc < 3) return;
    find_file(argv[2], 0, &filebuffer1);
    read_filedata(&filebuffer1, buffer1);
    if (filebuffer1.size_low > bit16max) {
        uint8_t* ptr = buffer1;
        for (uint32_t i = 0; i < filebuffer1.size_low / bit16max; i++) {
            play_pcm(22050, 8, ptr, 0xFFFF);
            wait_irq(5);
            ptr += bit16max;
        }

        if (filebuffer1.size_low % bit16max) {
            play_pcm(22050, 8, ptr, filebuffer1.size_low % bit16max);
            wait_irq(5);
        }
    } else {
        play_pcm(22050, 8, buffer1, filebuffer1.size_low);
    }
    return;
}