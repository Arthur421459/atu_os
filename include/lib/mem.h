#ifndef MEM_H
#define MEM_H
#include <stdint.h>
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
#endif