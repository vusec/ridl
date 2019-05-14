#include <cpuid.h>

void
cpuid(unsigned regs[4], unsigned leaf)
{
	__get_cpuid(leaf, regs + 0, regs + 1, regs + 2, regs + 3);
}

void
cpuidex(unsigned regs[4], unsigned leaf, unsigned subleaf)
{
	__get_cpuid_count(leaf, subleaf, regs + 0, regs + 1, regs + 2, regs + 3);
}
