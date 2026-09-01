#include <stddef.h>
#include <stdint.h>
#include <string.h>

void *memset(void *dest, int value, size_t size) {
    char v = (char)value;
    char* d = (char*)dest;
    for (size_t i = 0; i < size;i++) {
        d[i] = v;
    }
    return dest;
}


size_t strlen(const char *str) {
    size_t sz = 0;
    while (str[sz] != 0) sz++;
    return sz;
}

void *memcpy(void* restrict dest, const void* restrict src, size_t size) {
    char* d = dest;
    const char* s = src;
    for (size_t i = 0; i < size;i++) {
        d[i] = s[i];
    }
    return dest;
}

void *memmove(void *dest, const void *src, size_t size) {
    char* d = dest;
    const char* s = src;
    uintptr_t dp = (uintptr_t)dest;
    uintptr_t sp = (uintptr_t)src;
    if (dp < sp) {
        memcpy(dest, src, size);
    } else if (dp > sp) {
        for (size_t i = 0; i < size;i++) {
            d[size-1-i] = s[size - 1 - i];
        }
    }
    return dest;
}

char *strcpy(char *restrict dest, const char *src) {
    size_t i = 0;
    while (src[i] != 0) {
        dest[i] = src[i];
        i++;
    }
    dest[i] = 0;
    return dest;
}

char *strncpy(char* restrict dest, const char *src, size_t max) {
    size_t i = 0;
    while (src[i] != 0 && i < max) {
        dest[i] = src[i];
        i++;
    }
    while (i < max) {
        dest[i] = 0;
        i++;
    }
    return dest;
}

char *strcat(char *restrict dest, const char* restrict src) {
    size_t i = 0;
    size_t j = 0;
    while (dest[i] != 0) {
        i++;
    }
    while (src[j] != 0) {
        j++;
    }
    memcpy(dest+i, src, j);
    dest[i+j] = 0;
    return dest;
}

char *strncat(char *restrict dest, const char* restrict src, size_t max) {
    size_t i = 0;
    while (dest[i] != 0) {
        i++;
    }
    size_t j = 0;
    while (j < max && src[j] != 0) {
        dest[i + j] = src[j];
        j++;
    }
    dest[i + j] = 0;
    return dest;
}

int memcmp(const void *mem1, const void *mem2, size_t size) {
    const uint8_t* m1 = mem1;
    const uint8_t* m2 = mem2;
    for (size_t i = 0; i < size;i++) {
        if (m1[i] != m2[i]) {
            return (m1[i] < m2[i]) ? -1 : 1;
        }
    }
    return 0;
}

int strcmp(const char *str1, const char *str2) {
    size_t i = 0;
    while (str1[i] != 0 && str2[i] != 0) {
        if (str1[i] != str2[i]) {
            return (str1[i] < str2[i]) ? -1 : 1;
        }
        i++;
    }
    if (str1[i] != str2[i]) {
        return (str1[i] < str2[i]) ? -1 : 1;
    }
    return 0;
}

int strncmp(const char *str1, const char *str2, size_t max) {
    size_t i = 0;
    while (str1[i] != 0 && str2[i] != 0 && i < max) {
        if (str1[i] != str2[i]) {
            return (str1[i] < str2[i]) ? -1 : 1;
        }
        i++;
    }
    if (str1[i] != str2[i] && i < max) {
        return (str1[i] < str2[i]) ? -1 : 1;
    }
    return 0;
}

void *memchr(const void *mem, int value, size_t size) {
    const uint8_t* s = mem;
    uint8_t v = value;
    for (size_t i = 0; i < size;i++) {
        if (*s == v) {
            return (void*)s;
        }
        s++;
    }
    return NULL;
}

char *strchr(const char *str, int value) {
    char v = (char)value;

    while (*str != v) {
        if (*str == '\0') {
            return NULL;
        }
        str++;
    }

    return (char *)str;
}

size_t strcspn(const char *str, const char *list) {
    size_t result = (size_t)-1;
    while (*list != '\0') {
        const char *p = strchr(str, *list);
        if (p != NULL) {
            size_t r = (size_t)(p - str);

            if (r < result) result = r;
        }
        list++;
    }
    if (result == (size_t)-1) return strlen(str);

    return result;
}

char *strpbrk(const char *str, const char *list) {
    size_t i = strcspn(str, list);
    if (str[i] == '\0') return NULL;

    return (char *)str + i;
}

char *strrchr(const char *str, int value) {
    char v = (char)value;
    size_t sz = strlen(str);
    for (size_t i = sz+1; i > 0; i--) {
        if (str[i - 1] == v) return (char*)str+i-1;
    }

    return NULL;
}

size_t strspn(const char *str, const char *list) {
    size_t i = 0;
    while (str[i] != 0) {
        char equal = 0;
        size_t j = 0;
        while (list[j] != 0) {
            if (str[i] == list[j]) {
                equal = 1;
                break;
            }
            j++;
        }
        if (equal == 0) break;
        i++;
    }
    return i;
}

char* strstr(const char* str, const char* substr) {
    if (*substr == '\0') return (char*)str;
    if (strlen(str) < strlen(substr)) return NULL;
    while (*str != '\0') {
        if (*str == *substr) {
            const char* mstr = str;
            const char* sstr = substr;
            while (*mstr != '\0' && *sstr != '\0' && *mstr == *sstr) {
                mstr++; sstr++;
            }
            if (*sstr == '\0') {
                return (char*)str;
            }
        }
        str++;
    }
    return NULL;
}

char* strtokp = NULL;

char* strtok(char* restrict str, const char* restrict listdel) {
    char* ustr = str;
    if (ustr == NULL) ustr = strtokp;
    if (ustr == NULL) return NULL;
    ustr += strspn(ustr, listdel);
    if (*ustr == '\0') {
        strtokp = NULL;
        return NULL;
    }

    char* pointer = strpbrk(ustr, listdel);
    if (pointer == NULL) {
        strtokp = NULL;
        return ustr;
    }
    *pointer = '\0';
    pointer++;
    strtokp = pointer;
    return ustr;
}