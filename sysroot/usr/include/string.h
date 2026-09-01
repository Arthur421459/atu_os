#ifndef _STRING_H
#define _STRING_H

#define NULL 0
typedef unsigned int size_t;
// need strcoll and strxfrm

void *memset(void *dest, int value, size_t size);
void *memcpy(void* restrict dest, const void* restrict src, size_t size);
void *memmove(void *dest, const void *src, size_t size);

size_t strlen(const char *str);
char *strcpy(char *restrict dest, const char *src);
char *strncpy(char* restrict dest, const char *src, size_t max);
char *strcat(char *restrict dest, const char* restrict src);
char *strncat(char *restrict dest, const char* restrict src, size_t max);
int memcmp(const void *mem1, const void *mem2, size_t size);

#endif