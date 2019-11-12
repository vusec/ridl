#pragma once

#include <stdint.h>

#define force_inline __attribute__((always_inline)) inline

struct cpuid_regs {
	uint32_t eax, ebx, ecx, edx;
};

#define CPUID_SGX (1 << 2)
#define CPUID_RTM (1 << 11)
#define CPUID_L1D_FLUSH (1 << 28)

static force_inline void cpuid(struct cpuid_regs *regs)
{
	asm volatile("cpuid\n"
		: "=a" (regs->eax), "=b" (regs->ebx), "=c" (regs->ecx), "=d" (regs->edx)
		: "a" (regs->eax), "c" (regs->ecx));
}

static force_inline void cpuid_vendor(char buffer[12])
{
	struct cpuid_regs regs = {
		.eax = 0,
	};

	cpuid(&regs);

	*(uint32_t *)(buffer + 0) = regs.ebx;
	*(uint32_t *)(buffer + 4) = regs.edx;
	*(uint32_t *)(buffer + 8) = regs.ecx;
}

#define XBEGIN_INIT (~0u)

#define XABORT_EXPLICIT (1 << 0)
#define XABORT_RETRY    (1 << 1)
#define XABORT_CONFLICT (1 << 2)
#define XABORT_CAPACITY (1 << 3)
#define XABORT_DEBUG    (1 << 4)
#define XABORT_NESTED   (1 << 5)
#define XABORT_CODE(x)  (((x) >> 24) & 0xff)

static force_inline unsigned xbegin(void)
{
	uint32_t ret = XBEGIN_INIT;

	asm volatile(
		"xbegin 1f\n"
		"1:\n"
		: "+a" (ret)
		:: "memory");

	return ret;
}

static force_inline void xend(void)
{
	asm volatile("xend\n"
		::: "memory");
}

static force_inline uint64_t rdtsc(void)
{
	uint64_t lo, hi;

	asm volatile("rdtscp\n"
		: "=a" (lo), "=d" (hi)
		:: "%rcx");

	return (hi << 32) | lo;
}

static force_inline void clflush(volatile void *p)
{
	asm volatile("clflush (%0)\n"
		:: "r" (p));
}

static force_inline void clflushopt(volatile void *p)
{
	asm volatile("clflushopt (%0)\n"
		:: "r" (p));
}

static force_inline void lfence(void)
{
	asm volatile("lfence\n" ::: "memory");
}

static force_inline void sfence(void)
{
	asm volatile("sfence\n" ::: "memory");
}

static force_inline void mfence(void)
{
	asm volatile("mfence\n" ::: "memory");
}

static force_inline void movnti(volatile void *p, uint64_t val)
{
	asm volatile("movnti %1, (%0)\n"
		:: "r" (p), "r" (val));
}

static force_inline __int128 movntdqa(volatile void *p)
{
	__int128 ret;

	asm volatile("movntdqa (%1), %0\n"
		: "=x" (ret)
		: "r" (p));

	return ret;
}

