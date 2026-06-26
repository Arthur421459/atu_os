#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

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
FILE *hd;
uint32_t hdsize;
int main(int argc, char *argv[]) {
    if (argc < 2) {
        return 1;
    }
    hd = fopen(argv[1], "r+b");
    if (!hd) {
        perror("fopen");
        return 1;
    }
    fseek(hd, 0, SEEK_END);
    hdsize = ftell(hd);
    rewind(hd);

    struct mbrsector* sec = (struct mbrsector*)malloc(512);

    fread(sec, 1, 512, hd);

    if (argc >= 4) {
        if (strcmp(argv[2], "boot") == 0) {
            FILE *bootmbr = fopen(argv[3], "rb");
            buffer = (uint8_t*)malloc(512);
            fread(buffer, sizeof(uint8_t), 512, bootmbr);
            memcpy(sec->boot_code, buffer, sizeof(sec->boot_code));
            free(buffer);
        } else if (strcmp("active", argv[2]) == 0 && (atoi(argv[3]) - 1) < 4) {
            uint8_t partnum = atoi(argv[3]) - 1;
            sec->part[partnum].active = 0x80;
        } else if (strcmp("inactive", argv[2]) == 0 && (atoi(argv[3]) - 1) < 4) {
            uint8_t partnum = atoi(argv[3]) - 1;
            sec->part[partnum].active = 0;
        }
    } else {
        srand(time(NULL));
        sec->uid = rand();
    }
    sec->sig = 0xAA55;
    if (argc >= 5) {
        if (strcmp("part", argv[2]) == 0 && (atoi(argv[3]) - 1) < 4 && (atoi(argv[4]) < (int)((hdsize>>9)-2048) || atoi(argv[4]) == 0)) {
            uint8_t partnum = atoi(argv[3]) - 1;
            sec->part[partnum].parttype = 0;
            sec->part[partnum].active = 0;
            sec->part[partnum].firstlba = 2048;

            if (atoi(argv[4]) == 0) {
                sec->part[partnum].secamount = (hdsize>>9)-2049;
            } else {
                sec->part[partnum].secamount = atoi(argv[4]);
            }
            memset(sec->part[partnum].chsstart, 0xFF, 3);
            memset(sec->part[partnum].chsend, 0xFF, 3);
            sec->part[partnum].chsstart[0] = 0xFE;
            sec->part[partnum].chsend[0] = 0xFE;
        }
    }
    fseek(hd, 0, SEEK_SET);
    fwrite(sec, sizeof(struct mbrsector), 1, hd);
    free(sec);
    return 0;
}