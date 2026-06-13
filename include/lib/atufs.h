#ifndef ATUFS_H
#define ATUFS_H
#include <stdint.h>
struct atufs_info {
    uint16_t jmpormagic;
    uint16_t magic;
    uint8_t ver;
    uint16_t block_size;
    uint16_t file_size;
    uint16_t boot2_size;
    uint8_t skipped_blocks;
    uint32_t files;
    uint32_t clusters;
    uint32_t alocated_files;
    uint32_t alocated_clusters;
    uint16_t journal_size;
    uint32_t startbmpfile;
    uint32_t startbmpcluster;
    uint32_t file0;
    uint32_t cluster0;
    uint8_t label[8];
    uint8_t zero[460];
} __attribute__((packed));
struct extent {
    uint32_t startcluster;
    uint32_t manyclusters;
};
struct file {
    uint32_t size_low;
    uint16_t size_high;
    uint32_t last_mod;
    uint32_t last_access;
    uint32_t creation;
    uint16_t user_id;
    uint8_t attributes;
    uint8_t future;
    union {
        uint8_t data[490];
        struct extent extents[61];
    } __attribute__((packed));
} __attribute__((packed));
struct entry {
    uint32_t file;
    uint16_t entry_size;
    uint8_t atr;
    uint8_t namesize;
    uint8_t name[];
};
extern struct atufs_info atufsinfo;
extern uint16_t blockinsec;
uint64_t read_filedata(struct file* file1, uint8_t* buffer);
void find_file(const char* name, uint8_t* buffer, uint64_t buffer_size, struct file* f);

#endif