#include <stdlib.h>
#include <string.h>

#include <macros.h>

#include <info/cpuid.h>
#include <info/topology.h>

#include <vuln/ridl.h>

void
query_ridl_info(struct ridl_info *info)
{
	struct cpu_topology topo;
	unsigned regs[4];
	unsigned family, model, stepping;

	memset(info, 0, sizeof *info);

	read_cpu_topology(&topo);

	if (check_topology_smt(&topo)) {
		info->smt_vuln = 1;
	}

	info->md_clear = cpuid_has_feature("md_clear");

	if (cpuid_get_vendor_id() != CPUID_INTEL)
		return;

	cpuid(regs, 0x00000001);

	family = EXTRACT(regs[0], 8, 4) + EXTRACT(regs[0], 20, 8);
	stepping = EXTRACT(regs[0], 0, 4);
	model = (EXTRACT(regs[0], 16, 4) << 4) | EXTRACT(regs[0], 4, 4);

	if (family != 6)
		return;

	switch (model) {
	case 0x1a:
	case 0x1e:
	case 0x1f:
	case 0x25:
	case 0x2a:
	case 0x2c:
	case 0x2d:
	case 0x2e:
	case 0x2f:
	case 0x3a:
	case 0x3c:
	case 0x3e:
	case 0x3f:
	case 0x45:
	case 0x46:
	case 0x47:
	case 0x4f:
	case 0x4e:
	case 0x56:
	case 0x5e:
		info->mfbds = 1;
		info->msbds = 1;
		info->mlpds = 1;
		info->mdsum = 1;
		break;
	case 0x37:
	case 0x4a:
	case 0x4c:
	case 0x4d:
	case 0x57:
	case 0x5a:
	case 0x5d:
	case 0x65:
	case 0x6e:
	case 0x75:
	case 0x85:
		info->msbds = 1;
		break;
	case 0x55: {
		switch (stepping) {
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
			info->mfbds = 1;
			info->msbds = 1;
			info->mlpds = 1;
			info->mdsum = 1;
			break;
		default: break;
		}
	}
	case 0x8e: {
		switch (stepping) {
		case 9:
		case 10:
			info->mfbds = 1;
			info->msbds = 1;
			info->mlpds = 1;
			info->mdsum = 1;
			break;
		case 11:
			info->msbds = 1;
			info->mlpds = 1;
			info->mdsum = 1;
			break;
		default: break;
		}
	}
	case 0x9e: {
		switch (stepping) {
		case 9:
		case 10:
		case 11:
			info->mfbds = 1;
			info->msbds = 1;
			info->mlpds = 1;
			info->mdsum = 1;
			break;
		case 12:
			info->msbds = 1;
			info->mlpds = 1;
			info->mdsum = 1;
			break;
		default: break;
		}
	}
	default: break;
	}
}
