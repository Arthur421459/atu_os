#include "kernel/apic.h"
#include "lib/io.h"
// Only old PIC for now but in future, APIC! :D

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

void set_pit_freq(uint32_t freq) {
    uint16_t div = initialpic_freq / freq;
    outb(0x43, 0b00110111); // set command (channel 0, square wave)
    outb(0x40, div & 0xFF); // low
    outb(0x40, div >> 8); // high
}