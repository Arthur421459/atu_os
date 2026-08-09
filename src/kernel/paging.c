#include "kernel/paging.h"
#include "lib/bitmap.h"
#include "lib/main.h"
#include "lib/string.h"
extern uintptr_t ppalloc(uint32_t pages);
extern tuple we_palloc(uint32_t pages);
// pg dir types
// 0000 nothing
// 0010 kernel
// 0011 other system things
// 0100 user
// 0101 user alloc
// 0111 many types
// 1??? full
// pg table types
// 000 nothing
// 010 kernel
// 011 other system things
// 100 user
// 101 user alloc
void *phys_to_virt(uint32_t physpage, uint32_t pages, uint16_t flags) {
    uint32_t* directmapentries = directmap;
    bool a = false;
    uintptr_t counter = 0;
    uintptr_t ofs = 0;
    for (int i = 0; i < 0x10000; i++) {
        if (counter >= pages) break;
        if (!(directmapentries[i] & 1)) {
            if (!a) {
                ofs = i;
                a = true;
            }
            counter++;
        } else {
            counter = 0;
            a = false;
        }
    }
    if (counter < pages) return (void*)0;
    directmapentries += ofs;
    for (uintptr_t i = 0; i < pages;i++) {
        directmapentries[i] = (physpage+i) << 12 | flags;
        invlpg((void*)(0xE0000000+((ofs+i) << 12)));
    }
    return (void*)(0xE0000000+(ofs << 12));
}
void free_directmap(void *ptr, uint32_t pages) {
    uint32_t pagenum = ((uintptr_t)ptr - 0xE0000000) >> 12;
    uint32_t* directmapentries = directmap;
    directmapentries += pagenum;
    memset(directmapentries, global_page | page_writable, pages*sizeof(uint32_t));
    invlpgs(ptr, pages);
}

uintptr_t get_pag() {
    uintptr_t addr;
    asm volatile ("movl %%cr3, %0" : "=r"(addr) :: "memory");
    return addr;
}

void map_page(uint32_t* pdaddr, uint32_t physpage, uint32_t virtpage, uint32_t pages, uint16_t tflags, uint16_t dflags) {
    uintptr_t alocated = 0;
    uintptr_t olddirentry = 4096;
    uint32_t* tble;
    for (uint32_t i = 0; i < pages; i++) {
        uint32_t abspage = virtpage+i;
        uint32_t direntry = abspage >> 10;
        uint32_t tentry = abspage & 1023;

        if (direntry != olddirentry) {
            if (tble) {
                free_directmap(tble, 1);
                tble = 0;
            }
            if (!(pdaddr[direntry] & 1)) {
                uint32_t phystpg = ppalloc(1);
                pdaddr[direntry] = phystpg << 12 | dflags;
                tble = phys_to_virt(phystpg, 1, page_writable | page_present | sysmisc_pagetble);
                memset(tble, 0, 4096);
                tble[tentry] = (physpage+alocated) << 12 | tflags;
                alocated++;
                continue;
            }
            uint32_t phystble = pdaddr[direntry] & 0xFFFFF000;
            if (((tflags & osattr_masktble) / 2) != (pdaddr[direntry] & osattr_maskdir) && (pdaddr[direntry] & osattr_maskdir) != multiple_pagedir) {
                pdaddr[direntry] |= multiple_pagedir;
            }
            tble = phys_to_virt(phystble >> 12, 1, page_writable | page_present | sysmisc_pagetble);
            if (tble[tentry] & page_present) return;
            tble[tentry] = (physpage+alocated) << 12 | tflags;
            alocated++;
        } else {
            if (tble[tentry] & page_present) return;
            tble[tentry] = (physpage+alocated) << 12 | tflags;
            alocated++;
        }
    }
    if (tble) {
        free_directmap(tble, 1);
    }
    if ((uint32_t)pdaddr == 0xD0100000) {
        invlpgs((void*)(virtpage << 12), pages);
    }
}
void unmap_page(uint32_t* pdaddr, uint32_t first, uint32_t pages) {
    for (uint32_t i = 0; i < pages; i++) {
        uint32_t abspage = first+i;
        uint32_t direntry = abspage >> 10;
        uint32_t tentry = abspage & 1023;
        if (!(pdaddr[direntry] & 1)) {
            continue;
        }
        uint32_t phystble = pdaddr[direntry] & 0xFFFFF000;
        uint32_t* tble = phys_to_virt(phystble >> 12, 1, page_present | sysmisc_pagetble | page_writable);
        tble[tentry] = 0;
        free_directmap(tble, 1);
    }
    if ((uint32_t)pdaddr == 0xD0100000) {
        invlpgs((void*)(first << 12), pages);
    }
}
uintptr_t vpalloc(uint32_t pages) {
    return bmp_alloc((void*)0xD0000000, pages, 0xFFFFF);
}
tuple palloc_virt_and_phys(uintptr_t pages, uint16_t tflags, uint16_t dflags) {
    uintptr_t physpage = ppalloc(pages);
    uintptr_t virtpage = vpalloc(pages);
    map_page((uint32_t*)0xD0100000, physpage, virtpage, pages, tflags, dflags);
    tuple prog;
    prog.a = virtpage << 12;
    prog.b = physpage << 12;
    memset((void*)prog.a, 0, pages << 12);
    return prog;
}
tuple lpalloc_virt_and_phys(uintptr_t pages, uint16_t tflags, uint16_t dflags) {
    uintptr_t physpage = ppalloc(pages);
    uintptr_t virtpage = vpalloc(pages);
    map_page((uint32_t*)0xD0100000, physpage, virtpage, pages, tflags, dflags);
    tuple prog;
    prog.a = virtpage << 12;
    prog.b = physpage << 12;
    memset((void*)prog.a, 0, pages << 12);
    return prog;
}
void *palloc(uintptr_t pages, uint16_t tflags, uint16_t dflags) {
    uintptr_t virtpage = vpalloc(pages);
    uintptr_t physpage = ppalloc(pages);
    map_page((uint32_t*)0xD0100000, physpage, virtpage, pages, tflags, dflags);
    memset((void*)(virtpage << 12), 0, pages << 12);
    return (void*)(virtpage << 12);
}
void *lpalloc(uintptr_t pages, uint16_t tflags, uint16_t dflags) {
    uintptr_t virtpage = vpalloc(pages);
    uintptr_t physpage = ppalloc(pages);
    map_page((uint32_t*)0xD0100000, physpage, virtpage, pages, tflags, dflags);
    memset((void*)(virtpage << 12), 0, pages << 12);
    return (void*)(virtpage << 12);
}