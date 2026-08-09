#ifndef BITMAP_H
#define BITMAP_H
#include "lib/main.h"

void mark_true(void* bitmaptr, uint32_t first, uint32_t size);
void mark_false(void* bitmaptr, uint32_t first, uint32_t size);

uint32_t bmp_alloc(void *bitmaptr, uint32_t size, uint32_t bitmapsize);
tuple webmp_alloc(void *bitmaptr, uint32_t size, uint32_t bitmapsize);
#endif