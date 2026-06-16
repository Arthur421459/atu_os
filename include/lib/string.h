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
static inline void memcpy(void *restrict dest, const void *restrict src, uintptr_t n) {
    char *d = (char *)dest;
    const char *s = (const char *)src;
    for (uintptr_t i = 0; i < n; i++) {
        d[i] = s[i];
    }
}
static inline void memset(void *s, int c, uintptr_t n) {
    char *p = (char*)s;
    for (uintptr_t i = 0;i < n;i++) {
        p[i] = (unsigned char)c;
    }
}
static inline void num_to_str(uintptr_t num, char* str) {
    int i = 0;
    if (num == 0) {
        str[i++] = '0';
        str[i] = '\0';
        return;
    }

    while (num != 0) {
        char resultado = num % 10;
        resultado += '0';
        str[i++] = resultado;
        num /= 10;
    }

    str[i] = '\0';
    for (int start = 0, end = i - 1; start < end; start++, end--) {
        char temp = str[start];
        str[start] = str[end];
        str[end] = temp;
    }
}
#endif
