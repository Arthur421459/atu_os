#ifndef CORE_H
#define CORE_H
typedef enum {
    false = 0,
    true = 1
} bool;

#define false 0
#define true 1

typedef unsigned int size_t;

typedef unsigned char uint8_t;
typedef char int8_t;

typedef unsigned short uint16_t;
typedef short int16_t;

typedef unsigned long uint32_t;
typedef long int32_t;

typedef unsigned long long uint64_t;
typedef long long int64_t;

typedef unsigned int uintptr_t;
typedef int intptr_t;

typedef struct {
    uintptr_t a;
    uintptr_t b;
} tuple;

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