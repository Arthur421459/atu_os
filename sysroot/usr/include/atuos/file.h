#ifndef FILE_H
#define FILE_H
#include <atuos/core.h>
struct extent {
    uint32_t startcluster;
    uint32_t manyclusters;
} __attribute__((packed));

struct file {
    uint32_t size_low;
    uint16_t size_high;
    uint32_t last_mod;
    uint32_t last_access;
    uint32_t creation;
    uint16_t user_id;
    uint8_t attributes;
    uint8_t future;
    uint16_t padding;
    union {
        uint8_t data[488];
        struct extent extents[61];
    } __attribute__((packed));
} __attribute__((packed));
struct entry {
    uint32_t file;
    uint16_t entry_size;
    uint8_t atr;
    uint8_t namesize;
    uint8_t name[];
} __attribute__((packed));

uint64_t read_filedata(struct file* file1, uint8_t* buffer);
uint32_t find_file(const char* name, uint32_t root, struct file* f);

#endif