#include <atuos/syscall.h>
#include <stdint.h>
uintptr_t syscall_support = 0;
extern void sysentercall(void* result, size_t eax, size_t ebx, size_t ecx, size_t edx, void* esi, void* edi);
syscallreturn atuos_syscall(size_t eax, size_t ebx, size_t ecx, size_t edx, void* esi, void* edi) {
    syscallreturn result = {0};
    switch (syscall_support) {
        case 1:
            sysentercall(&result, eax, ebx, ecx, edx, esi, edi);
        break;
        default:
            asm volatile ("int $0xA7" : "=a"(result.eax), "=b"(result.ebx), "=c"(result.ecx), "=d"(result.edx), "=S"(result.esi), "=D"(result.edi) :
            "a"(eax), "b"(ebx), "c"(ecx), "d"(edx), "S"(esi), "D"(edi) : "memory", "cc");
        break;
    }
    return result;
}
void syscall_init() {
    if (syscall_support != 0) return;
    syscallreturn a = atuos_syscall(0, 0, 0, 0, 0, 0);
    syscall_support = a.eax;
}