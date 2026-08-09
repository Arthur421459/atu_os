#ifndef HEAP_H
#define HEAP_H
#include "lib/main.h"
#include "kernel/bootinfo.h"
extern uint32_t uraminbytes;
extern uint32_t traminbytes;
extern uint32_t traminpages;
extern uintptr_t kernel_reserved_end;

void mark_used(uint32_t first_page, uint32_t total);
void mark_free(uint32_t first_page, uint32_t total);

uintptr_t ppalloc(uint32_t pages);
tuple we_ppalloc(uint32_t pages);

void init_heap(struct smap* smaps, int total_smaps);

void *malloc(size_t bytes, uint16_t tflags, uint16_t dflags);
void *amalloc(size_t bytes, uint16_t tflags, uint16_t dflags);

void *lmalloc(size_t bytes, uint16_t tflags, uint16_t dflags);
void *lamalloc(size_t bytes, uint16_t tflags, uint16_t dflags);

void free(void* ptr);
#endif