#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "asm-tools.h"

int main(void)
{
	char vendor[12];
	struct cpuid_regs regs;

	cpuid_vendor(vendor);

	if (memcmp(vendor, "GenuineIntel", sizeof vendor) != 0)
		return -1;

	regs.eax = 0x00000007;
	regs.ecx = 0;
	cpuid(&regs);

	if (!(regs.ebx & CPUID_SGX))
		return -1;

	return 0;
}
