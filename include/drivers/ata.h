#ifndef ATA_H
#define ATA_H
#include <stdint.h>
extern void read_sector(uint32_t lba, uint16_t* buffer, uint8_t sectors);
#endif