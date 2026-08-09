#include <stdint.h>
#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
struct atufs_info {
    uint16_t jmpormagic;
    uint16_t magic;
    uint8_t ver;
    uint16_t block_size;
    uint16_t file_size;
    uint16_t boot2_size;
    uint8_t skipped_blocks;
    uint32_t files;
    uint32_t clusters;
    uint32_t alocated_files;
    uint32_t alocated_clusters;
    uint16_t journal_size;
    uint32_t startbmpfile;
    uint32_t startbmpcluster;
    uint32_t file0;
    uint32_t cluster0;
    uint8_t label[8];
    uint8_t zero[458];
} __attribute__((packed));
struct extent {
    uint32_t startcluster;
    uint32_t manyclusters;
} __attribute__((packed));

struct f {
    uint32_t size_low;
    uint16_t size_high;
    uint32_t last_mod;
    uint32_t last_access;
    uint32_t creation;
    uint16_t user_id;
    uint8_t attributes;
    uint8_t future;
    uint16_t padding;
    union {
        uint8_t data[488];
        struct extent extents[61];
    } __attribute__((packed));
} __attribute__((packed));
struct entry {
    uint32_t file;
    uint16_t entry_size;
    uint8_t atr;
    uint8_t namesize;
    uint8_t name[];
} __attribute__((packed));


uint64_t hdsize;
uint64_t boot2size;
FILE *hd;
FILE *boot2;
FILE *boot1;
uint32_t partstart;
struct tuple {
    uint32_t a;
    uint32_t b;
};
struct part {
    uint8_t active;
    uint8_t chsstart[3];
    uint8_t parttype;
    uint8_t chsend[3];
    uint32_t firstlba;
    uint32_t secamount;
} __attribute__((packed));
struct mbrsector {
    uint8_t boot_code[442];
    uint32_t uid;
    struct part part[4];
    uint16_t sig;
} __attribute__((packed));
uint8_t* buffer;
struct tuple fixclusters(uint32_t totalbase)
{
    struct tuple ret;

    while (true) {
        uint32_t bitmap = (totalbase + 4095) / 4096;
        uint32_t clusters = totalbase - bitmap;

        if (clusters + ((clusters + 4095) / 4096) == totalbase) {
            ret.a = clusters;
            ret.b = bitmap;
            return ret;
        }

        totalbase = clusters;
    }
}
struct atufs_info generate_0info(bool jmpormagic) {
    struct atufs_info atufsinfo = {0};
    if (jmpormagic) {
        atufsinfo.jmpormagic = 0x32EB;
    } else {
        atufsinfo.jmpormagic = 0xF5A7;
    }
    atufsinfo.magic = 0xF5A7;
    atufsinfo.ver = 0;
    atufsinfo.block_size = 0x200;
    atufsinfo.file_size = 0x200;
    atufsinfo.boot2_size = 128;
    atufsinfo.skipped_blocks = 2;
    atufsinfo.files = hdsize / 16384 - 1;

    atufsinfo.journal_size = 2;
    atufsinfo.startbmpfile = 2 + atufsinfo.boot2_size + atufsinfo.skipped_blocks;
    atufsinfo.startbmpcluster = atufsinfo.startbmpfile + (atufsinfo.files + 4095) / 4096;

    atufsinfo.clusters = ((hdsize + 511) / 512) - 
    (atufsinfo.startbmpcluster +
    atufsinfo.journal_size +
    atufsinfo.files);

    atufsinfo.clusters -= (atufsinfo.clusters + 4095) / 4096;
    struct tuple a1 = fixclusters(atufsinfo.clusters);
    atufsinfo.clusters = a1.a;
    
    atufsinfo.file0 = atufsinfo.startbmpcluster + a1.b + atufsinfo.journal_size;
    atufsinfo.cluster0 = atufsinfo.file0 + atufsinfo.files;
    atufsinfo.alocated_files = 0;
    atufsinfo.alocated_clusters = 0;
    memcpy(atufsinfo.label, "ATUOS   ", 8);
    return atufsinfo;
}
int main(int argc, char *argv[]) {
    hd = fopen(argv[1], "r+b");
    if (!hd) {
        perror("fopen hd");
        return 1;
    }
    if (argc < 3) {
        fseek(hd, 0, SEEK_END);
        hdsize = ftell(hd);
        rewind(hd);
        // create info
        struct atufs_info atuinfo = generate_0info(true);
        printf("Clusters in MB: %i\n", atuinfo.clusters/2048);
        printf("total clusters: %i\n", atuinfo.clusters);
        printf("total files: %i\n", atuinfo.files);
        fwrite(&atuinfo, sizeof(struct atufs_info), 1, hd);

        atuinfo = generate_0info(false);
        fwrite(&atuinfo, sizeof(struct atufs_info), 1, hd);

        // boot2
        if (argc > 2) {
            boot2 = fopen(argv[2], "rb");

            fseek(boot2, 0, SEEK_END);
            boot2size = ftell(boot2);
            rewind(boot2);

            buffer = (uint8_t*)malloc(boot2size + 1);
            fread(buffer, 1, boot2size, boot2);
            fwrite(buffer, 1, boot2size, hd);
            free(buffer);
            fclose(boot2);
        }
        if (argc > 3) {
            boot1 = fopen(argv[3], "rb");
            fseek(boot1, 54, SEEK_SET);
            fseek(hd, 54, SEEK_SET);
            buffer = (uint8_t*)malloc(512);
            fread(buffer, 1, 512-54, boot1);
            fwrite(buffer, 1, 512-54, hd);
        }


        // alocate first file (main root)
        fseek(hd, atuinfo.startbmpfile*512, SEEK_SET);
        uint8_t a = 1;
        fwrite(&a, 1, 1, hd);

        // create main root
        fseek(hd, atuinfo.file0*512, SEEK_SET);
        struct f root = {0};
        root.attributes = 1;
        root.size_low = 1;
        root.size_high = 0;
        root.creation = (uint32_t)time(NULL);
        root.last_access = (uint32_t)time(NULL);
        root.last_mod = (uint32_t)time(NULL);
        root.future = 0;
        root.user_id = 0;
        fwrite(&root, sizeof(struct f), 1, hd);
        return 0;
    } else if (argc > 3 && strcmp("part", argv[2]) == 0 && (atoi(argv[3]) - 1) < 4) {
        uint8_t partnum = atoi(argv[3]) - 1;
        struct mbrsector mbrsec = {0};
        rewind(hd);
        fread(&mbrsec, sizeof(struct mbrsector), 1, hd);
        struct part* part = &mbrsec.part[partnum];
        part->parttype = 0xA7; // atufs part type
        
        rewind(hd);
        fwrite(&mbrsec, 512, 1, hd);

        uint64_t part_offset = (uint64_t)part->firstlba * 512;
        fseek(hd, part_offset, SEEK_SET);

        hdsize = part->secamount*512;

        struct atufs_info atuinfo = generate_0info(true);
        printf("Clusters in MB: %i\n", atuinfo.clusters/2048);
        printf("total clusters: %i\n", atuinfo.clusters);
        printf("total files: %i\n", atuinfo.files);
        fwrite(&atuinfo, sizeof(struct atufs_info), 1, hd);
        atuinfo = generate_0info(false);
        fwrite(&atuinfo, sizeof(struct atufs_info), 1, hd);

        // boot2
        if (argc > 4) {
            boot2 = fopen(argv[4], "rb");

            fseek(boot2, 0, SEEK_END);
            boot2size = ftell(boot2);
            rewind(boot2);

            buffer = (uint8_t*)malloc(boot2size + 1);
            fread(buffer, 1, boot2size, boot2);
            fwrite(buffer, 1, boot2size, hd);
            free(buffer);
            fclose(boot2);
        }
        if (argc > 5) {
            boot1 = fopen(argv[5], "rb");
            fseek(boot1, 54, SEEK_SET);
            fseek(hd, part_offset+54, SEEK_SET);
            buffer = (uint8_t*)malloc(512);
            fread(buffer, 1, 458, boot1);
            fwrite(buffer, 1, 458, hd);
        }

        // alocate first file (main root)
        fseek(hd, part_offset+atuinfo.startbmpfile*512, SEEK_SET);
        uint8_t a = 1;
        fwrite(&a, 1, 1, hd);

        // create main root
        fseek(hd, part_offset+atuinfo.file0*512, SEEK_SET);
        struct f root = {0};
        root.attributes = 1;
        root.size_low = 0;
        root.size_high = 0;
        root.creation = (uint32_t)time(NULL);
        root.last_access = (uint32_t)time(NULL);
        root.last_mod = (uint32_t)time(NULL);
        root.future = 0;
        root.user_id = 0;
        fwrite(&root, sizeof(struct f), 1, hd);
        return 0;
    }
    return 0;
}