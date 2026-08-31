#include <atuos/file.h>

uint32_t find_file(const char* name, uint32_t root, struct file* f) {
    uint32_t filenum = 0;
    asm volatile ("int $0xA7" : "=a"(filenum) : "a"(10), "b"(root), "S"(name), "D"(f) : "memory", "cc");
    return filenum;
}
uint64_t read_filedata(struct file *file1, uint8_t *buffer) {
    uint32_t size_low;
    uint16_t size_high;
    asm volatile ("int $0xA7" : "=a"(size_low), "=b"(size_high) : "a"(11), "S"(file1), "D"(buffer): "memory", "cc");
    return ((uint64_t)(size_high) << 32) | size_low; 
}
