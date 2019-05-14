#pragma once

enum {
	CPUID_EAX = 0,
	CPUID_EBX,
	CPUID_ECX,
	CPUID_EDX,
};

enum {
	CPUID_UNKNOWN,
	CPUID_AMD,
	CPUID_INTEL,
};

void
cpuid(unsigned regs[4], unsigned leaf);
void
cpuidex(unsigned regs[4], unsigned leaf, unsigned subleaf);
int
cpuid_get_vendor_id(void);
char *
cpuid_get_vendor(void);
char *
cpuid_get_brand_string(void);
const char *
cpuid_get_codename(void);
int
cpuid_has_feature(const char *name);
