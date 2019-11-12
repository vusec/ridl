#include <cpuid.h>

void
cpuid(unsigned regs[4], unsigned leaf)
{
	__get_cpuid(leaf, regs + 0, regs + 1, regs + 2, regs + 3);
}

void
cpuidex(unsigned regs[4], unsigned leaf, unsigned subleaf)
{
#if __GNUC__ > 5
	__get_cpuid_count(leaf, subleaf, regs + 0, regs + 1, regs + 2, regs + 3);
#else
	asm volatile(
		"cpuid\n"
		: "=a" (regs[0]), "=b" (regs[1]), "=c" (regs[2]), "=d" (regs[3])
		: "a" (regs[0]), "c" (regs[2]));
#endif
}
