#ifndef PAGING_H
#define PAGING_H
#include "lib/main.h"
#define no_cache           (1 << 4)
#define pwt_enable         (1 << 3)
#define user_page          (1 << 2)
#define page_writable      (1 << 1)
#define page_present          1
#define global_page        (1 << 8)

#define kernel_pagedir     (2 << 8)
#define sysmisc_pagedir    (3 << 8)
#define prog_pagedir       (4 << 8)
#define progalloc_pagedir  (5 << 8)
#define progstack_pagedir  (6 << 8)
#define multiple_pagedir   (7 << 8)
#define full_pagedir       (1 << 6)
#define osattr_maskdir     (15 << 8)

#define kernel_pagetble    (2 << 9)
#define sysmisc_pagetble   (3 << 9)
#define prog_pagetble      (4 << 9)
#define progalloc_pagetble (5 << 9)
#define progstack_pagetble (6 << 9)
#define osattr_masktble    (7 << 9)

extern uintptr_t page_directory[1024];
extern uint32_t directmap[131072];

struct malloc_header {
  uint32_t total_pages;
  uint32_t total_bytes;
  uint32_t phys_page;
  uint32_t virt_page;
} __attribute__((packed));

#define physvirtdiff (0xC0000000 - 0x100000)
#define pgmask 0xfffff000

void *phys_to_virt(uint32_t physpage, uint32_t pages, uint16_t flags);
void free_directmap(void *ptr, uint32_t pages);

uintptr_t get_pag();

void map_page(uint32_t* pdaddr, uint32_t physpage, uint32_t virtpage, uint32_t pages, uint16_t tflags, uint16_t dflags);
void unmap_page(uint32_t* pdaddr, uint32_t first, uint32_t pages);

uintptr_t vpalloc(uint32_t pages);
tuple palloc_virt_and_phys(uintptr_t pages, uint16_t tflags, uint16_t dflags);
void *palloc(uintptr_t pages, uint16_t tflags, uint16_t dflags);

tuple lpalloc_virt_and_phys(uintptr_t pages, uint16_t tflags, uint16_t dflags);
void *lpalloc(uintptr_t pages, uint16_t tflags, uint16_t dflags);

static inline void invlpg(void *addr) {
    asm volatile("invlpg (%0)" : : "r" (addr) : "memory");
}
static inline void invlpgs(void *start_addr, size_t num_pages) {
    char *ptr = (char *)start_addr;
    for (size_t i = 0; i < num_pages; i++) {
        invlpg(ptr);
        ptr += 4096;
    }
}

#endif