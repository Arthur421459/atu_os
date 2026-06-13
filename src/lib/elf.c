#include <stdint.h>
#include "lib/elf.h"
#include "lib/bool.h"
#include "lib/string.h"
#include "lib/mem.h"
bool is_elf(uint8_t* buffer) {
    struct elf_header* elfh = (struct elf_header*)buffer;
    if (elfh->magic != 0x7F) {return false;} // check magic code
    if (!(cmpstr("ELF", (char*)elfh->elf_ascii))) {return false;} // check signature
    return true;
}
bool is_compatible(uint8_t* buffer) {
    if (!(is_elf(buffer))) {return false;}
    struct elf_header* elfh = (struct elf_header*)buffer;
    if (elfh->bits != 1) {return false;} // check 32 bits
    if (elfh->endian != 1) {return false;} // check endian (correct is little endian)
    if (elfh->elfh_ver != 1) {return false;} // check header ver
    if (elfh->elf_ver != 1) {return false;} // check elf ver
    if (elfh->arch != 0x03) {return false;} // check arch (correct is x86 or i386)
    return true;
}
uintptr_t load_elf(uint8_t* buffer) {
    if (!(is_compatible(buffer))) {return 0;}
    struct elf_header* elfh = (struct elf_header*)buffer;
    struct ph_entry* ph_entries = (struct ph_entry*)(buffer+elfh->pheader_ofs);
    for (int i = 0; i < elfh->entrynum_ph; i++) {
        struct ph_entry entry = ph_entries[i];
        if (entry.seg_type != 1) {continue;} // se não for PT_LOAD então pule
        memcpy((void*)entry.p_vaddr, buffer+entry.p_offset, entry.p_filesz); // copiar para o endereço correto
        memset((void*)(entry.p_vaddr+entry.p_filesz), 0, entry.p_memsz - entry.p_filesz); // preencher o bss
    }
    return elfh->pentry_ofs; // return offset
}