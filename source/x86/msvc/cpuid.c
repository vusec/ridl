#include <intrin.h>

void
cpuid(unsigned regs[4], unsigned leaf)
{
	__cpuid(regs, leaf);
}

void
cpuidex(unsigned regs[4], unsigned leaf, unsigned subleaf)
{
	__cpuidex(regs, leaf, subleaf);
}
