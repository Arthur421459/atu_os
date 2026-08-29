#include "kernel/cpuid.h"
#include "kernel/heap.h"
#include "kernel/msr.h"
#include "lib/main.h"
#include "kernel/bootinfo.h"
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

#define physvirtdiff (0xC0000000 - 0x100000)
extern char bss_start;
extern char bss_end;
extern char ldheap_start[];

void clear_bss() {
    uint8_t *p = (uint8_t*)&bss_start;
    for (uintptr_t i = 0; i < (uintptr_t)&bss_end - (uintptr_t)&bss_start; i++) {
        p[i] = 0;
    }
}
uintptr_t page_directory[1024] __attribute__((aligned(4096)));
uint32_t directmap[131072];
uint32_t d0pgtable[1024];
uint32_t kernelpgtble[16384];
uint32_t traminbytes;
uint32_t uraminbytes;
uint32_t traminpages;
uint32_t heap_size;
uint32_t heap_sizebits;
uint8_t* heap_start;
uint32_t kernel_size;
uint32_t kernel_sizepg;
struct boot_info* binfo;
void calculateram() {
    for (int i = 0; i < binfo->total_smaps;i++) {
        traminbytes += binfo->smaps[i].length;
        if (binfo->smaps[i].type == 1) {
            uraminbytes += binfo->smaps[i].length;
        }
    }
}
void init_heap_pt1() {
    heap_start = (uint8_t*)ldheap_start;
    traminpages = traminbytes >> 12;
    heap_size = (traminpages+7) >> 3;
    kernel_size = (uintptr_t)heap_start + heap_size - 0xC0000000 + 4095;
    kernel_sizepg = (kernel_size + 4095) >> 12;
}
void initmap_page(void* addr, uint16_t flags, uintptr_t pages, uintptr_t firstphys) {
    uintptr_t* pointer = addr;
    for (uintptr_t i = 0; i < pages;i++) {
        pointer[i] = ((firstphys+i) << 12) | flags;
    }
}
void config_paging() {
    // map the page dir pointers
    // 0xC0000000 kernel
    initmap_page(page_directory+768, page_present | page_writable | kernel_pagedir  | full_pagedir, (kernel_sizepg + 1023) >> 10, ((uintptr_t)&kernelpgtble - physvirtdiff) >> 12);
    // 0xD0000000 some info
    initmap_page(page_directory+832, page_present | page_writable | sysmisc_pagedir | full_pagedir, 1, ((uintptr_t)&d0pgtable - physvirtdiff) >> 12);
    // 0xE0000000 to 0xF0000000 direct map
    initmap_page(page_directory+896, page_present | page_writable | sysmisc_pagedir | full_pagedir, 128, ((uintptr_t)&directmap - physvirtdiff) >> 12);

    // map page table
    initmap_page(kernelpgtble, page_present | page_writable | kernel_pagetble | global_page, kernel_sizepg, 0x100);
    for (int i = 0; i < 131072; i++) {
        directmap[i] = page_writable | sysmisc_pagetble; // set basic directmap
    }
    uintptr_t pagedir_phys = (uintptr_t)&page_directory - physvirtdiff;
    initmap_page(d0pgtable, page_present | page_writable | sysmisc_pagetble, 1, pagedir_phys >> 12);
    asm volatile ("movl %0, %%cr3" :: "r"(pagedir_phys) : "memory");
    // secure paging!
}
void afterkinit(void* bbinfo) {
    binfo = (struct boot_info*)bbinfo;
    calculateram();
    init_heap_pt1();
    config_paging();
    clear_bss();
    // first part is done!
}
// now that kernel is totally mapped, you can init another things :)
#include "drivers/ata.h"
#include "kernel/gdt.h"
#include "kernel/idt.h"
#include "kernel/apic.h"
#include "lib/atufs.h"
#include "kernel/paging.h"
struct boot_info* vbinfo;
extern void syscall_enter();
extern uint32_t stack_top;
void set_sysenter() {
    // detect sysenter
    struct cpuid_result a = cpuid(1, 0);
    if (!(a.edx & CPUID_FEATURE_SYSENTER)) return; // no sysenter :(

    // sysenter :D
    wrmsr(IA32_SYSENTER_CS, kernelcode_seg);
    wrmsr(IA32_SYSENTER_EIP, (uint64_t)((uint32_t)&syscall_enter));
    wrmsr(IA32_SYSENTER_ESP, stack_top);

}
void msr_init() {
    set_sysenter();
}
void afterpaging() {
    config_gdt();
    vbinfo = phys_to_virt((uintptr_t)binfo >> 12, 1, page_present | page_writable);
    void* ptr1 = phys_to_virt((uintptr_t)vbinfo->partaddr >> 12, 1, page_present | page_writable);
    set_partstart(ptr1);
    free_directmap(ptr1, 1);
    remap_pic(0x20, 0x28);
    config_idt();
    uintptr_t physsmaps = (uintptr_t)vbinfo->smaps;
    ptr1 = phys_to_virt(physsmaps >> 12, (vbinfo->total_smaps + 99) / 100, page_present | page_writable);
    init_heap(ptr1+(physsmaps & 4095), vbinfo->total_smaps);
    free_directmap(ptr1, (vbinfo->total_smaps + 99) / 100);
    set_pit_freq(100);
    init_atufs();
    free_directmap(vbinfo, 1);
    if (has_msr()) {
        msr_init();
    }
}