#ifndef _SYSCALL_H
#define _SYSCALL_H
#include <stddef.h>
#include <stdint.h>

typedef struct {
    size_t eax;
    size_t ebx;
    size_t ecx;
    size_t edx;
    void* esi;
    void* edi;
} syscallreturn;

void syscall_init();
syscallreturn atuos_syscall(size_t eax, size_t ebx, size_t ecx, size_t edx, void* esi, void* edi);
void atuos_syscall_noret(size_t eax, size_t ebx, size_t ecx, size_t edx, void* esi, void* edi) __attribute__((noreturn));
#endif