#include "lib/bitmap.h"
#include "lib/main.h"
void mark_true(void *bitmaptr, uint32_t first, uint32_t size) {
    uint8_t* ptr = (uint8_t*)bitmaptr;
    for (uint32_t i = first; i < (first+size);i++) {
        uint32_t byte = i >> 3;
        uint32_t localb = i & 7;

        ptr[byte] |= 1 << localb;
    }
}

void mark_false(void *bitmaptr, uint32_t first, uint32_t size) {
    uint8_t* ptr = (uint8_t*)bitmaptr;
    for (uint32_t i = first; i < (first+size);i++) {
        uint32_t byte = i >> 3;
        uint32_t localb = i & 7;

        ptr[byte] &= ~(1 << localb);
    }
}

uint32_t bmp_alloc(void *bitmaptr, uint32_t size, uint32_t bitmapsize) {
    uint8_t* ptr = (uint8_t*)bitmaptr; 
    uintptr_t result = 0;
    uintptr_t counter = 0;
    for (uintptr_t i = 0;i < bitmapsize;i++) {
        if (counter >= size) break;
        uint32_t byte = i >> 3;
        uint32_t localb = i & 7;
        if (!(ptr[byte] & (1 << localb))) {
            if (!counter) {
                result = i;
            }
            counter++;
        } else {
            counter = 0;
        }
    }
    if (counter != size) return 0xFFFFFFFF;
    mark_true(bitmaptr, result, size);
    return result;
}

tuple webmp_alloc(void *bitmaptr, uint32_t size, uint32_t bitmapsize) {
    uint8_t* ptr = (uint8_t*)bitmaptr; 
    tuple result = {0};
    bool a = false;
    uintptr_t counter = 0;
    for (uintptr_t i = 0;i < bitmapsize;i++) {
        if (counter >= size) break;
        uint32_t byte = i >> 3;
        uint32_t localb = i & 7;
        if (!(ptr[byte] & (1 << localb))) {
            if (!a) {
                result.a = i;
                a = true;
            }
            counter++;
        } else if (!a) {
            continue;
        } else break;
    }
    result.b = counter;
    mark_true(bitmaptr, result.a, result.b);
    return result;
}