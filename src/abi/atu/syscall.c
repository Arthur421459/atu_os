#include <atuos/syscall.h>
#include <stdint.h>
uintptr_t syscall_support = 0;
syscallreturn atuos_syscall(size_t eax, size_t ebx, size_t ecx, size_t edx, void* esi, void* edi) {
    syscallreturn result = {0};
    void* sysenterret;
    sysenterret = &&aftersysenter;
    switch (syscall_support) {
        case 1:
            asm volatile (
            "pushl %0\n"
            "pushl %1\n"
            "movl %%esp, %%edx\n"
            "sysenter\n" : "=a"(result.eax), "=b"(result.ebx), "=c"(result.ecx), "=d"(result.edx), "=S"(result.esi), "=D"(result.edi) :
            "g"(ecx), "g"(edx), "a"(eax), "b"(ebx), "c"(sysenterret), "S"(esi), "D"(edi) : "memory", "cc");
            aftersysenter:;
        break;
        default:
            asm volatile ("int $0xA7" : "=a"(result.eax), "=b"(result.ebx), "=c"(result.ecx), "=d"(result.edx), "=S"(result.esi), "=D"(result.edi) :
            "a"(eax), "b"(ebx), "c"(ecx), "d"(edx), "S"(esi), "D"(edi) : "memory", "cc");
        break;
    }
    return result;
}
void syscall_init() {
    syscallreturn a = atuos_syscall(0, 0, 0, 0, 0, 0);
    syscall_support = a.eax;
}