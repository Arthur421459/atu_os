#ifndef MSR_H
#define MSR_H
#include "lib/main.h"
#include "kernel/cpuid.h"
// msr
#define IA32_SYSENTER_CS  0x174
#define IA32_SYSENTER_ESP 0x175
#define IA32_SYSENTER_EIP 0x176
static inline uint64_t rdmsr(uint32_t msr)
{
    uint32_t lo;
    uint32_t hi;

    asm volatile (
        "rdmsr"
        : "=a"(lo), "=d"(hi)
        : "c"(msr)
    );

    return ((uint64_t)hi << 32) | lo;
}

static inline void wrmsr(uint32_t msr, uint64_t value)
{
    uint32_t lo = (uint32_t)value;
    uint32_t hi = (uint32_t)(value >> 32);

    asm volatile (
        "wrmsr"
        :
        : "c"(msr), "a"(lo), "d"(hi)
        : "memory"
    );
}
static inline bool has_msr() {
    struct cpuid_result r = cpuid(1, 0);
    return (r.edx & CPUID_FEATURE_MSR) != 0;
}
#endif