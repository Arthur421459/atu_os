#ifndef CPUID_H
#define CPUID_H
#include "lib/main.h"
// edx
#define CPUID_FEATURE_MSR (2 << 5)
#define CPUID_FEATURE_SYSENTER (2 << 11)
struct cpuid_result {
    uint32_t eax;
    uint32_t ebx;
    uint32_t ecx;
    uint32_t edx;
};
static inline struct cpuid_result cpuid(uint32_t leaf, uint32_t subleaf)
{
    struct cpuid_result r;

    asm volatile (
        "cpuid"
        : "=a"(r.eax), "=b"(r.ebx), "=c"(r.ecx), "=d"(r.edx)
        : "a"(leaf), "c"(subleaf)
    );

    return r;
}
#endif