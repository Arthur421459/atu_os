#include "lib/atufs.h"
#include "drivers/ata.h"
#include "lib/io.h"
#include "lib/string.h"
#include <stdint.h>
struct atufs_info atufsinfo;
uint16_t blockinsec = 1;
extern volatile char* tvideo;
void init_atufs() {
    read_sector(0, (uint16_t*)&atufsinfo, 1);
    blockinsec = atufsinfo.block_size / 512;
}
uint64_t read_filedata(struct file* file1, uint8_t* buffer) {
    if (file1->attributes & 0b10000000) {
        uint64_t size;
        for (uint32_t i = 0; i < 61; i++) {
            struct extent e = file1->extents[i];
            uint32_t totalcsectors = e.manyclusters*blockinsec;
            if (e.manyclusters < 1) {
                break;
            }
            if (totalcsectors > 255) {
                uint32_t sector = atufsinfo.cluster0+(e.startcluster*blockinsec);
                for (uint32_t i = 0; i < (totalcsectors / 255);i++) {
                    read_sector(sector, (uint16_t*)buffer, 255);
                    sector += 255;
                    buffer += atufsinfo.block_size*255;
                }
                if (e.manyclusters % 255) {
                    read_sector(sector, (uint16_t*)buffer, totalcsectors % 255);
                }
            } else {
                read_sector(atufsinfo.cluster0+(e.startcluster*blockinsec), (uint16_t*)buffer, totalcsectors);
            }
        }
        size = ((uint64_t) file1->size_high << 32) | file1->size_low;
        return size;
    } else {
        memcpy(buffer, file1->data, file1->size_low);
        return (uint64_t)file1->size_low;
    }
    return 0;
}
void find_file(const char* name, uint8_t* buffer, uint64_t buffer_size, struct file* f) {
    uint64_t offset = 0;
    while (offset < buffer_size) {
        struct entry* fentry = (struct entry*)(buffer+offset);
        if (cmpstr_limit((char*)fentry->name, name, fentry->namesize)) {
            read_sector(atufsinfo.file0+(fentry->file), (uint16_t*)f, 1);
            return;
        }
        offset += fentry->entry_size;
    };
}