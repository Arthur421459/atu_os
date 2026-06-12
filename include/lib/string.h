#ifndef STRING_H
#define STRING_H
#include "lib/bool.h"
#include <stdint.h>
static inline int cmpstr(const char* a, const char* b) {
    while (*a && *b) {
        if (*a != *b)
            return false;
        a++;
        b++;
    }
    return true;
}
static inline int cmpstr_limit(const char* a, const char* b, int limit) {
    for (int i = 0; i < limit; i++) {
        if (a[i] != b[i]) {
            return false;
        }
    }
    return true;
}
static inline char* u8_str_limit(uint8_t* str, int size) {
    str[size] = '\0';
    return (char*)str;
}

static inline char captalize_letter(unsigned char letter) {
    if ((letter >= 'A' && letter <= 'Z') || (letter >= 'a' && letter <= 'z')) {
        return letter ^ 0x20;
    } else {
        return letter;
    }
}
#endif
