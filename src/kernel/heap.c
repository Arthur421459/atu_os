#include "kernel/heap.h"
#include "lib/string.h"
#include "kernel/bootinfo.h"
#include "lib/bitmap.h"
#include "kernel/paging.h"


extern char ldheap_start[];
extern uint8_t* heap_start;
extern uint32_t heap_size;

void mark_used(uint32_t first_page, uint32_t total) {
    for (uint32_t i = first_page; i < (first_page+total) && i < traminpages;i++) {
        uint32_t byte = i >> 3;
        uint32_t localb = i & 7;
        heap_start[byte] |= (1 << localb);
    }
}
void mark_free(uint32_t first_page, uint32_t total) {
    for (uint32_t i = first_page; i < (first_page+total) && i < traminpages;i++) {
        uint32_t byte = i >> 3;
        uint32_t localb = i & 7;
        heap_start[byte] &= ~(1 << localb);
    }
}
uintptr_t ppalloc(uint32_t pages) {
    uint32_t pagenum = bmp_alloc(heap_start, pages, heap_size);
    if (pagenum == 0) return 0;
    mark_used(pagenum, pages);
    pagenum += 0x100;
    
    return pagenum;
}
tuple we_ppalloc(uint32_t pages) {
    tuple result = {0};
    result = webmp_alloc(heap_start, pages, heap_size);
    if (result.b == 0) return result;
    mark_used(result.a, pages);
    result.a += 0x100;
    
    return result;
}
uintptr_t kernel_reserved_end;

void init_heap(struct smap* smaps, int total_smaps) {
    memset(heap_start, 0xFF, heap_size);
    for (int i = 0; i < total_smaps;i++) {
        if (smaps[i].base_addr < 0x100000) continue;
        if (smaps[i].type == 1) {
            mark_free((smaps[i].base_addr - 0x100000) >> 12,
            (smaps[i].length + 4095) >> 12);
        }
    }
    kernel_reserved_end = (uintptr_t)heap_start + heap_size;

    mark_used(0,(kernel_reserved_end - 0x100000 + 4095) >> 12);
}

void *malloc(size_t bytes, uint16_t tflags, uint16_t dflags) {
    uint32_t total_pages = (bytes+sizeof(struct malloc_header)+4095) >> 12;
    tuple prog = palloc_virt_and_phys(total_pages, tflags, dflags);
    uint8_t* ptr = (uint8_t*)prog.a;
    struct malloc_header* header = (struct malloc_header*)ptr;
    header->total_bytes = bytes+sizeof(struct malloc_header);
    header->total_pages = total_pages;
    header->phys_page = prog.b >> 12;
    header->virt_page = prog.a >> 12;
    ptr += sizeof(struct malloc_header);
    return (void*)ptr;
}

void *amalloc(size_t bytes, uint16_t tflags, uint16_t dflags) {
    tuple prog = palloc_virt_and_phys((bytes+8191) >> 12, tflags, dflags);
    uint8_t* ptr = (void*)prog.a;
    ptr += 4096-sizeof(struct malloc_header);
    struct malloc_header* header = (struct malloc_header*)ptr;
    header->total_bytes = bytes+sizeof(struct malloc_header);
    header->total_pages = (bytes+sizeof(struct malloc_header)+4095) >> 12;
    header->phys_page = (prog.b >> 12) + 1;
    header->virt_page = (prog.a >> 12) + 1;
    ptr += sizeof(struct malloc_header);
    return (void*)ptr;
}

void *lmalloc(size_t bytes, uint16_t tflags, uint16_t dflags) {
    uint32_t total_pages = (bytes+sizeof(struct malloc_header)+4095) >> 12;
    tuple prog = lpalloc_virt_and_phys(total_pages, tflags, dflags);
    uint8_t* ptr = (uint8_t*)prog.a;
    struct malloc_header* header = (struct malloc_header*)ptr;
    header->total_bytes = bytes+sizeof(struct malloc_header);
    header->total_pages = total_pages;
    header->phys_page = prog.b >> 12;
    header->virt_page = prog.a >> 12;
    ptr += sizeof(struct malloc_header);
    return (void*)ptr;
}

void *lamalloc(size_t bytes, uint16_t tflags, uint16_t dflags) {
    tuple prog = lpalloc_virt_and_phys((bytes+8191) >> 12, tflags, dflags);
    uint8_t* ptr = (void*)prog.a;
    ptr += 4096-sizeof(struct malloc_header);
    struct malloc_header* header = (struct malloc_header*)ptr;
    header->total_bytes = bytes+sizeof(struct malloc_header);
    header->total_pages = (bytes+sizeof(struct malloc_header)+4095) >> 12;
    header->phys_page = (prog.b >> 12) + 1;
    header->virt_page = (prog.a >> 12) + 1;
    ptr += sizeof(struct malloc_header);
    return (void*)ptr;
}
void free(void* ptr) {
    uint8_t* cptr = ptr;
    uint32_t pgnum = (uint32_t)cptr >> 12;
    struct malloc_header* header = (struct malloc_header*)(cptr-sizeof(struct malloc_header));
    memset(header, 0, header->total_bytes);
    unmap_page((void*)0xD0100000, pgnum, header->total_pages);
}
