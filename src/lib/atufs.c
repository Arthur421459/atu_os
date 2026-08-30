#include "lib/atufs.h"
#include "drivers/ata.h"
#include "drivers/cmos.h"
#include "kernel/paging.h"
#include "lib/string.h"
#include "lib/main.h"
#include "kernel/heap.h"
struct atufs_info atufsinfo;
uint16_t blockinsec = 1;

extern nixt worldtime;
void init_atufs() {
    read_sector_part(0, (uint16_t*)&atufsinfo, 1);
    blockinsec = atufsinfo.block_size / 512;
}
void clear_clusters(uint32_t firstcluster, uint32_t clusteramount) {
    // status = 0 delete
    // status = 1 alocate
    if (clusteramount == 0) {return;}
    if (atufsinfo.alocated_clusters < clusteramount) {return;}
    uint8_t* buffer = malloc(512, page_present | page_writable, page_present | page_writable);
    uint32_t currentsector = -1;
    for (uint32_t i = 0; i < clusteramount;i++) {
        uint32_t bit    = firstcluster + i;
        uint32_t local  = bit & 4095;
        uint32_t sector = bit >> 12;

        uint32_t byte   = local >> 3;
        uint32_t localb = local & 7;
        
        if (sector != currentsector) {
            if (currentsector != (uint32_t)-1) {
                write_sector_part(atufsinfo.startbmpcluster+currentsector, (uint16_t*)buffer, 1);
            }
            read_sector_part(atufsinfo.startbmpcluster+sector, (uint16_t*)buffer, 1);
            currentsector = sector;
        }
        buffer[byte] &= ~(1 << localb);
    }
    atufsinfo.alocated_clusters -= clusteramount;
    write_sector_part(0, (uint16_t*)&atufsinfo, 1);
    write_sector_part(atufsinfo.startbmpcluster+currentsector, (uint16_t*)buffer, 1);
    free(buffer);
}
struct extent alocate_clusters(uint32_t amount) {
    uint32_t alocated = 0;
    struct extent ext = {0};
    if (amount == 0) {return ext;}
    uint32_t currentsector = -1;
    uint8_t* buffer = malloc(512, page_present | page_writable, page_present | page_writable);
    for (uint32_t i = 0; i < atufsinfo.clusters;i++) {
        if (alocated >= amount) {break;}
        uint32_t sector = i >> 12;
        uint32_t local = i & 4095;
        uint32_t byte = local >> 3;
        uint32_t localb = local & 7;
        if (sector != currentsector) {
            if (currentsector != (uint32_t)-1) {
                write_sector_part(atufsinfo.startbmpcluster+currentsector, (uint16_t*)buffer, 1);
            }
            read_sector_part(atufsinfo.startbmpcluster+sector, (uint16_t*)buffer, 1);
            currentsector = sector;
        }
        if (!(buffer[byte] & (1 << localb))) {
            if (ext.manyclusters == 0) {
                ext.startcluster = i;
            }
            buffer[byte] |= (1 << localb);
            alocated++;
            ext.manyclusters++;
            atufsinfo.alocated_clusters++;
        } else if (ext.manyclusters == 0) {
            continue;
        } else {
            break;
        }
    }
    write_sector_part(0, (uint16_t*)&atufsinfo, 1);
    write_sector_part(atufsinfo.startbmpcluster+currentsector, (uint16_t*)buffer, 1);
    free(buffer);
    return ext;
}
void write_file(struct file* f, uint32_t filenum, uint8_t *buffer, uint64_t buffer_size, nixt time) {
    struct file copy = *f;
    uint64_t oldfilesize_inby = ((uint64_t) f->size_high << 32) | f->size_low;
    if (buffer_size <= 488) {
        if (copy.attributes & 0x80) {
            for (int i = 0; i < 61;i++) {
                struct extent e = copy.extents[i];
                clear_clusters(e.startcluster, e.manyclusters);
            }
        }
        copy.attributes &= 0x7F;
        memset(copy.data, 0, 488);
        memcpy(copy.data, buffer, (uint32_t)buffer_size);
    } else {
        if (!(copy.attributes & 0x80)) {
            memset(copy.data, 0, 488);
        }
        copy.attributes |= 0x80;
        uint32_t nfilesize_inclus = (uint32_t)((buffer_size+511) >> 9);
        uint32_t oldfilesize_inclus = (uint32_t)((oldfilesize_inby+511) >> 9);
        if (nfilesize_inclus == oldfilesize_inclus) {
            for (int i = 0; i < 61; i++) {
                struct extent e = copy.extents[i];
                if (!e.manyclusters) break;
                lwrite_sector_part(atufsinfo.cluster0+e.startcluster, (uint16_t*)buffer, e.manyclusters);
            }
        } else {
            uint32_t clusters_left = nfilesize_inclus;
            for (int i = 0; i < 61;i++) {
                struct extent e = copy.extents[i];
                if (clusters_left == 0) {
                    if (!(e.manyclusters)) {break;}
                    clear_clusters(e.startcluster, e.manyclusters);
                    memset(&copy.extents[i], 0, sizeof(struct extent));
                    continue;
                }
                if (e.manyclusters) {
                    if (e.manyclusters > clusters_left) {
                        clear_clusters(e.startcluster+clusters_left, e.manyclusters - clusters_left);
                        for (int j = i + 1; j < 61; j++) {
                            clear_clusters(copy.extents[j].startcluster, copy.extents[j].manyclusters);
                            memset(&copy.extents[j], 0, sizeof(struct extent));
                        }
                        copy.extents[i].manyclusters = clusters_left;
                        lwrite_sector_part(atufsinfo.cluster0+e.startcluster, (uint16_t*)buffer, clusters_left);
                        break;
                    } else if (e.manyclusters == clusters_left){
                        lwrite_sector_part(atufsinfo.cluster0+e.startcluster, (uint16_t*)buffer, clusters_left);
                        break;
                    } else {
                        lwrite_sector_part(atufsinfo.cluster0+e.startcluster, (uint16_t*)buffer, e.manyclusters);
                        buffer += e.manyclusters*512;
                        clusters_left -= e.manyclusters;
                    }
                } else {
                    copy.extents[i] = alocate_clusters(clusters_left);
                    if (copy.extents[i].manyclusters == 0) {return;}
                    lwrite_sector_part(atufsinfo.cluster0+copy.extents[i].startcluster, (uint16_t*)buffer, copy.extents[i].manyclusters);
                    buffer += copy.extents[i].manyclusters * 512;
                    clusters_left -= copy.extents[i].manyclusters;
                }
            }
        }
    }
    copy.last_access = time;
    copy.last_mod = time;
    copy.size_low = (uint32_t)buffer_size;
    copy.size_high = (buffer_size >> 32) & 0xFFFF;
    write_sector_part(atufsinfo.file0+filenum, (uint16_t*)&copy, 1);
}
uint64_t read_filedata(struct file* file1, uint8_t* buffer) {
    if (file1->attributes & 0b10000000) {
        uint64_t size;
        uint8_t* start_buffer = buffer;
        for (uint32_t i = 0; i < 61; i++) {
            struct extent e = file1->extents[i];
            uint32_t totalcsectors = e.manyclusters*blockinsec;
            if (e.manyclusters < 1) {
                break;
            }
            lread_sector_part(atufsinfo.cluster0+(e.startcluster*blockinsec), (uint16_t*)buffer, totalcsectors);
            buffer += totalcsectors * 512;
        }
        size = ((uint64_t) file1->size_high << 32) | file1->size_low;
        start_buffer[size] = '\0';
        return size;
    } else {
        memcpy(buffer, file1->data, file1->size_low);
        buffer[file1->size_low] = '\0';
        return (uint64_t)file1->size_low;
    }
    return 0;
}
uintptr_t find_file(const char* name, uint8_t* buffer, uint64_t buffer_size, struct file* f) {
    uint64_t offset = 0;
    while (offset < buffer_size) {
        struct entry* fentry = (struct entry*)(buffer+offset);
        if (fentry->entry_size == 0) break;
        if (cmpstr_limit((char*)fentry->name, name, fentry->namesize)) {
            read_sector_part(atufsinfo.file0+(fentry->file), (uint16_t*)f, 1);
            return fentry->file;
        }
        offset += fentry->entry_size;
    };
    return 0;
}
void create_entry(uint32_t file, uint32_t root, uint8_t* name, uint8_t type, uint8_t name_size) {
    struct file* rootfile = malloc(sizeof(struct file), page_present | page_writable, page_present | page_writable);
    read_sector_part(atufsinfo.file0+root, (uint16_t*)rootfile, 1);
    uint64_t rootdatasize = ((rootfile->size_low+8+name_size+511) >> 9) << 9;
    uint8_t* rootdata = malloc(rootdatasize, page_present | page_writable, page_present | page_writable);
    memset(rootdata, 0, rootdatasize);
    uint64_t size = read_filedata(rootfile, rootdata);
    if (size == 0) {
        memset(rootdata, 0, 512);
    }
    uint64_t finalsize = size;
    uint64_t offset = 0;
    while (offset < rootdatasize) {
        struct entry* fentry = (struct entry*)(rootdata+offset);
        if (fentry->atr == 0) {
            if (offset+fentry->entry_size > rootdatasize) {return;}
            if (fentry->entry_size < name_size+8 && fentry->entry_size != 0) {
                offset += fentry->entry_size;
                continue;
            }
            if (fentry->entry_size == 0) {
                fentry->entry_size = ((name_size+7) >> 3) << 3;
                fentry->entry_size += 8;
                finalsize += fentry->entry_size;
            }
            fentry->namesize = name_size;
            fentry->atr = type;
            fentry->file = file;
            memcpy(fentry->name, name, name_size);
            write_file(rootfile, root, rootdata, finalsize, worldtime);
            break;
        }
        offset += fentry->entry_size;
    }
    free(rootdata);
    free(rootfile);
}
void delete_entry(uint32_t root, const char* name, uint8_t name_size) {
    struct file* rootfile = malloc(sizeof(struct file), page_present | page_writable, page_present | page_writable);
    read_sector_part(atufsinfo.file0+root, (uint16_t*)rootfile, 1);
    uint64_t rootdatasize = ((rootfile->size_low+1+511)/512)*512;
    uint8_t* rootdata = malloc(rootdatasize, page_present | page_writable, page_present | page_writable);
    uint64_t size = read_filedata(rootfile, rootdata);
    uint64_t offset = 0;
    while (offset < rootdatasize) {
        struct entry* fentry = (struct entry*)(rootdata+offset);
        if (cmpstr_limit((char*)fentry->name, name, name_size)) {
            fentry->atr = 0;
            memset(fentry->name, 0, fentry->namesize);
            break;
        }
        offset += fentry->entry_size;
    }
    write_file(rootfile, root, rootdata, size, worldtime);
    free(rootdata);
    free(rootfile);
}
void rename_entry(uint32_t root, const char* oldname, const char* newname, uint8_t newnamesize) {
    struct file* rootfile = malloc(sizeof(struct file), page_present | page_writable, page_present | page_writable);
    read_sector_part(atufsinfo.file0+root, (uint16_t*)rootfile, 1);
    uint64_t rootdatasize = ((rootfile->size_low+1+511)/512)*512;
    uint8_t* rootdata = malloc(rootdatasize, page_present | page_writable, page_present | page_writable);
    memset(rootdata, 0, rootdatasize);
    uint64_t size = read_filedata(rootfile, rootdata);
    uint64_t offset = 0;
    while (offset < rootfile->size_low) {
        struct entry* fentry = (struct entry*)(rootdata+offset);
        if (cmpstr_limit((char*)fentry->name, oldname, fentry->namesize)) {
            if (fentry->namesize >= newnamesize) {
                fentry->namesize = newnamesize;
                memset(fentry->name, 0,fentry->namesize);
                memcpy(fentry->name, newname, newnamesize);
                write_file(rootfile, root, rootdata, size, worldtime);
            } else {
                write_file(rootfile, root, rootdata, size, worldtime);
                create_entry(fentry->file, root, (uint8_t*)newname, fentry->atr, newnamesize);
                memset(fentry->name, 0, fentry->namesize);
                fentry->atr = 0;
            }
            free(rootdata);
            free(rootfile);
            return;
        }
        offset += fentry->entry_size;
    }
    free(rootdata);
    free(rootfile);
}

uint32_t alocate_file() {
    uint32_t currentsector = -1;
    uint16_t* buffer = malloc(512, page_present | page_writable, page_present | page_writable);
    for (uint32_t i = 0; i < atufsinfo.files;i++) {
        uint32_t sector = i >> 12;
        uint32_t local = i & 4095;
        uint32_t byte = local >> 3;
        uint32_t localb = local & 7;
        if (sector != currentsector) {
            read_sector_part(atufsinfo.startbmpfile+sector, buffer, 1);
            currentsector = sector;
        }
        if (!(buffer[byte] & (1 << localb))) {
            buffer[byte] |= (1 << localb);
            write_sector_part(atufsinfo.startbmpfile+sector, buffer, 1);
            atufsinfo.alocated_files++;
            write_sector_part(0, (uint16_t*)&atufsinfo, 1);
            free(buffer);
            return i;
        }
    }
    free(buffer);
    return 0;
}
void delete_file(uint32_t filen) {
    uint32_t sector = filen >> 12;
    uint32_t local = filen & 4095;
    uint32_t byte = local >> 3;
    uint32_t localb = local & 7;
    uint16_t* buffer = malloc(512, page_present | page_writable, page_present | page_writable);
    read_sector_part(atufsinfo.startbmpfile+sector, buffer, 1);
    if (!( ((uint8_t*)buffer)[byte] & (1 << localb))) {free(buffer); return;}

    ((uint8_t*)buffer)[byte] &= ~(1 << localb);
    atufsinfo.alocated_files--;
    write_sector_part(atufsinfo.startbmpfile+sector, buffer, 1);
    write_sector_part(0, (uint16_t*)&atufsinfo, 1);

    read_sector_part(atufsinfo.file0+filen, buffer, 1);
    struct file* file = (struct file*)buffer;
    if (file->size_high || file->size_low > 488) {
        for (int i = 0; i < 61;i++) {
            struct extent e = file->extents[i];
            if (!(e.manyclusters)) {break;}
            clear_clusters(e.startcluster, e.manyclusters);
        }
    }
    memset(buffer, 0, 0x200);
    write_sector_part(atufsinfo.file0+filen, buffer, 1);
}

uint32_t create_file(uint8_t* buffer, uint64_t buffer_size, nixt time, uint16_t userid, uint8_t attr) {
    uint32_t filenum = alocate_file();
    if (filenum == 0) {
        return 1;
    }
    struct file file = {0};
    file.size_low = 0;
    file.size_high = 0;
    file.user_id = userid;
    file.creation = time;
    file.last_access = time;
    file.last_mod = time;
    if (buffer_size > 488)
        attr |= 0x80;
    else
        attr &= ~0x80;
    file.attributes = attr;
    file.future = 0;
    write_file(&file, filenum, buffer, buffer_size, time);
    return filenum;
}