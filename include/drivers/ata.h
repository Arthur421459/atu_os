#ifndef ATA_H
#define ATA_H
#include <stdint.h>
void read_sector(uint32_t lba, uint16_t* buffer, uint8_t sectors);
void write_sector(uint32_t lba, uint16_t* buffer, uint8_t sectors);

void lread_sector(uint32_t lba, uint16_t* buffer, uint32_t sectors);
void lwrite_sector(uint32_t lba, uint16_t* buffer, uint32_t sectors);

void read_sector_part(uint32_t lba, uint16_t *buffer, uint8_t sectors);
void write_sector_part(uint32_t lba, uint16_t *buffer, uint8_t sectors);

void lread_sector_part(uint32_t lba, uint16_t *buffer, uint32_t sectors);
void lwrite_sector_part(uint32_t lba, uint16_t *buffer, uint32_t sectors);

void set_partstart(void* partaddr);
#endif