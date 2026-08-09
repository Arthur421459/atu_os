#ifndef APIC_H
#define APIC_H
#include "lib/main.h"

#define MasterPIC_code 0x20
#define SlavePIC_code 0xA0
#define MasterPIC_data 0x21
#define SlavePIC_data 0xA1

#define initialpic_freq 1193182

void remap_pic(uint8_t master_ofs, uint8_t slave_ofs);
void set_pit_freq(uint32_t freq);

#endif