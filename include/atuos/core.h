#ifndef CORE_H
#define CORE_H
#include "lib/main.h"
struct malloc_header {
  uint32_t total_pages;
  uint32_t total_bytes;
  uint32_t phys_page;
  uint32_t virt_page;
} __attribute__((packed));
void wait_irq(uint8_t irq);
void *malloc(size_t bytes);
void free(void* ptr);
void print_wpos(const char* str, int pos);
#endif