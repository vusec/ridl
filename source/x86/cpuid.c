#include <stdlib.h>
#include <string.h>

#include <macros.h>

#include <info/cpuid.h>

#define AMD_FAMILY_K8        0x0f
#define AMD_FAMILY_K10       0x10
#define AMD_FAMILY_K8L       0x11
#define AMD_FAMILY_FUSION    0x12
#define AMD_FAMILY_BOBCAT    0x14
#define AMD_FAMILY_BULLDOZER 0x15
#define AMD_FAMILY_JAGUAR    0x16
#define AMD_FAMILY_ZEN       0x17

int
cpuid_get_vendor_id(void)
{
	char vendor[13];
	unsigned regs[4];

	cpuid(regs, 0);
	memcpy(vendor, regs + 1, 4);
	memcpy(vendor + 4, regs + 3, 4);
	memcpy(vendor + 8, regs + 2, 4);
	vendor[12] = '\0';

	if (strcmp(vendor, "GenuineIntel") == 0)
		return CPUID_INTEL;

	if (strcmp(vendor, "AuthenticAMD") == 0)
		return CPUID_AMD;

	return CPUID_UNKNOWN;
}

char *
cpuid_get_vendor(void)
{
	char vendor[13];
	unsigned regs[4];

	cpuid(regs, 0);
	memcpy(vendor, regs + 1, 4);
	memcpy(vendor + 4, regs + 3, 4);
	memcpy(vendor + 8, regs + 2, 4);
	vendor[12] = '\0';

	return strdup(vendor);
}

char *
cpuid_get_brand_string(void)
{
	char name[49];
	unsigned level;
	unsigned regs[4];

	for (level = 0; level < 3; ++level) {
		cpuid(regs, 0x80000002 + level);
		memcpy(name + level * 16, regs + 0, 4);
		memcpy(name + level * 16 + 4, regs + 1, 4);
		memcpy(name + level * 16 + 8, regs + 2, 4);
		memcpy(name + level * 16 + 12, regs + 3, 4);
	}

	name[48] = '\0';

	return strdup(name);
}

const char *
amd_get_codename(void)
{
	unsigned regs[4];
	unsigned family, model;

	cpuid(regs, 0x00000001);

	family = EXTRACT(regs[0], 8, 4) + EXTRACT(regs[0], 20, 8);
	model = (EXTRACT(regs[0], 16, 4) << 4) | EXTRACT(regs[0], 4, 4);

	switch (family) {
	case AMD_FAMILY_K8: return "Hammer";
	case AMD_FAMILY_K10: return "K10";
	case AMD_FAMILY_K8L: return "Llano (K8L)";
	case AMD_FAMILY_BOBCAT: return "Desna/Ontario/Zacate (Bobcat)";
	case AMD_FAMILY_BULLDOZER: {
		switch (model) {
		case 0x00:
		case 0x01:
		case 0x02: return "Orochi (Bulldozer)";
		case 0x10: return "Trinity (Piledriver)";
		case 0x13: return "Richland (Piledriver)";
		case 0x30: return "Kaveri (Steamroller)";
		case 0x38: return "Godavari (Steamroller)";
		case 0x60: return "Carrizo (Excavator)";
		case 0x65: return "Carrizo-L/Bristol Ridge (Excavator)";
		case 0x70: return "Stoney Ridge (Excavator)";
		default: break;
		}
	} break;
	case AMD_FAMILY_JAGUAR: {
		switch (model) {
		case 0x00: return "Kabini/Temash (Jaguar)";
		case 0x30: return "Beema/Mullins (Puma)";
		default: break;
		}
	} break;
	case AMD_FAMILY_ZEN: {
		switch (model) {
		case 0x00:
		case 0x01: return "Zeppelin (Zen)";
		case 0x08: return "Pinnacle Ridge (Zen+)";
		case 0x10:
		case 0x11: return "Raven Ridge (Zen)";
		case 0x20: return "Valhalla (Zen 2)";
		default: break;
		}
	}
	default: break;
	}

	return "Unknown";
}

const char *intel_codenames[] = {
	[0x1a] = "Nehalem",
	[0x1d] = "Dunnington",
	[0x1e] = "Nehalem",
	[0x1f] = "Nehalem",
	[0x25] = "Westmere",
	[0x2a] = "Sandy Bridge",
	[0x2c] = "Westmere",
	[0x2d] = "Sandy Bridge",
	[0x2e] = "Nehalem",
	[0x2f] = "Westmere",
	[0x37] = "Baytrail",
	[0x3a] = "Ivy Bridge",
	[0x3c] = "Haswell",
	[0x3d] = "Broadwell",
	[0x3e] = "Ivy Bridge",
	[0x3f] = "Haswell",
	[0x45] = "Haswell",
	[0x46] = "Haswell",
	[0x47] = "Broadwell",
	[0x4d] = "Avoton",
	[0x4e] = "Skylake",
	[0x4f] = "Broadwell",
	[0x56] = "Broadwell",
	[0x5c] = "Apollo Lake",
	[0x5e] = "Skylake",
	[0x5f] = "Denverton",
	[0x66] = "Cannon Lake",
};

const char *
intel_get_codename(void)
{
	unsigned model, stepping;
	unsigned regs[4];

	cpuid(regs, 0x00000001);

	stepping = EXTRACT(regs[0], 0, 4);
	model = (EXTRACT(regs[0], 16, 4) << 4) | EXTRACT(regs[0], 4, 4);

	switch (model) {
	case 0x55: {
		switch (stepping) {
		case 0:
		case 1:
		case 2:
		case 3:
		case 4: return "Skylake SP";
		case 7: return "Cascade Lake";
		default: return "Unknown";
		}
	}
	case 0x8e: {
		switch (stepping) {
		case 9: return "Kaby Lake";
		case 10: return "Coffee Lake";
		case 11:
		case 12: return "Whiskey Lake";
		default: return "Unknown";
		}
	}
	case 0x9e: {
		switch (stepping) {
		case 9: return "Kaby Lake";
		case 10:
		case 11: return "Coffee Lake";
		case 12: return "Coffee Lake Refresh";
		case 13: return "Whiskey Lake";
		default: return "Unknown";
		}
	}
	default: break;
	}

	if (intel_codenames[model])
		return intel_codenames[model];

	return "Unknown";
}

const char *
cpuid_get_codename(void)
{
	char *vendor = cpuid_get_vendor();
	const char *codename;

	if (strcmp(vendor, "AuthenticAMD") == 0) {
		codename = amd_get_codename();
	} else if (strcmp(vendor, "GenuineIntel") == 0) {
		codename = intel_get_codename();
	} else {
		codename = "Unknown";
	};

	free(vendor);

	return codename;
}

struct cpuid_feature {
	const char *name;
	unsigned leaf, subleaf, reg, bit;
};

struct cpuid_feature features[] = {
	{ "pcid",       0x01, 0, CPUID_ECX, 17 },

	{ "sgx",        0x07, 0, CPUID_EBX,  2 },
	{ "hle",        0x07, 0, CPUID_EBX,  4 },
	{ "avx2",       0x07, 0, CPUID_EBX,  5 },
	{ "smep",       0x07, 0, CPUID_EBX,  7 },
	{ "invpcid",    0x07, 0, CPUID_EBX, 10 },
	{ "rtm",        0x07, 0, CPUID_EBX, 11 },
	{ "mpx",        0x07, 0, CPUID_EBX, 14 },
	{ "avx512f",    0x07, 0, CPUID_EBX, 16 },
	{ "avx512dq",   0x07, 0, CPUID_EBX, 17 },
	{ "smap",       0x07, 0, CPUID_EBX, 20 },
	{ "avx512ifma", 0x07, 0, CPUID_EBX, 21 },
	{ "clflushopt", 0x07, 0, CPUID_EBX, 23 },
	{ "clwb",       0x07, 0, CPUID_EBX, 24 },
	{ "avx512pf",   0x07, 0, CPUID_EBX, 26 },
	{ "avx512er",   0x07, 0, CPUID_EBX, 27 },
	{ "avx512cd",   0x07, 0, CPUID_EBX, 28 },
	{ "sha",        0x07, 0, CPUID_EBX, 29 },
	{ "avx512bw",   0x07, 0, CPUID_EBX, 30 },
	{ "avx512vl",   0x07, 0, CPUID_EBX, 31 },

	{ "md_clear",  0x07, 0, CPUID_EDX, 10 },
	{ "bit13",     0x07, 0, CPUID_EDX, 13 },
	{ "ibpb",      0x07, 0, CPUID_EDX, 26 },
	{ "ibrs",      0x07, 0, CPUID_EDX, 26 },
	{ "stibp",     0x07, 0, CPUID_EDX, 27 },
	{ "l1d_flush", 0x07, 0, CPUID_EDX, 28 },
	{ "arch_caps", 0x07, 0, CPUID_EDX, 29 },
	{ "ssbd",      0x07, 0, CPUID_EDX, 31 },

	{ NULL, 0, 0, 0, 0 },
};

struct cpuid_feature amd_features[] = {
	{ "ibpb",         0x80000008, 0, CPUID_EBX, 12 },
	{ "ibrs",         0x80000008, 0, CPUID_EBX, 14 },
	{ "stibp",        0x80000008, 0, CPUID_EBX, 15 },
	{ "always_ibrs",  0x80000008, 0, CPUID_EBX, 16 },
	{ "always_stibp", 0x80000008, 0, CPUID_EBX, 17 },
	{ "prefer_ibrs",  0x80000008, 0, CPUID_EBX, 18 },
	{ "ssbd",         0x80000008, 0, CPUID_EBX, 24 },
	{ "vm_ssbd",      0x80000008, 0, CPUID_EBX, 25 },
	{ "no_ssb",       0x80000008, 0, CPUID_EBX, 26 },
	{ NULL, 0, 0, 0, 0 },
};

int
amd_has_feature(const char *name)
{
	unsigned regs[4];
	struct cpuid_feature *feature;

	for (feature = amd_features; feature->name; ++feature) {
		if (strcmp(feature->name, name) != 0)
			continue;

		cpuidex(regs, feature->leaf, feature->subleaf);

		return !!(regs[feature->reg] & (1 << feature->bit));
	}

	return 0;
}

int
cpuid_has_feature(const char *name)
{
	char *vendor = cpuid_get_vendor();
	unsigned regs[4];
	struct cpuid_feature *feature;
	int ret = 0;

	if (strcmp(vendor, "AuthenticAMD") == 0) {
		ret = amd_has_feature(name);
	}

	free(vendor);

	if (ret)
		return ret;

	for (feature = features; feature->name; ++feature) {
		if (strcmp(feature->name, name) != 0)
			continue;

		cpuidex(regs, feature->leaf, feature->subleaf);

		return !!(regs[feature->reg] & (1 << feature->bit));
	}

	return 0;
}
