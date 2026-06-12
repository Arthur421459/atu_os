#ifndef ATUFS_H
#define ATUFS_H
#include <stdint.h>
typedef struct {
    
} __attribute__((packed)) file_entry;
typedef struct {
    uint16_t start;
    uint32_t size;
} __attribute__((packed)) extent;
file_entry find_file(const char* filename, const char* ext);
void read_file(file_entry file, uintptr_t offset);
int fread_file(const char* filename, const char* ext, uintptr_t offset);
#endif