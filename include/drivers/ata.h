#ifndef ATA_H
#define ATA_H
#include <stdint.h>
void read_sector(uint32_t lba, uint16_t* buffer, uint8_t sectors);
void write_sector(uint32_t lba, uint16_t* buffer, uint8_t sectors);
void lread_sector(uint32_t lba, uint16_t* buffer, uint32_t sectors);
void lwrite_sector(uint32_t lba, uint16_t* buffer, uint32_t sectors);
#endif