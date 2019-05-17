#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <info/cpuid.h>

#include <vuln/spectre.h>

void
query_spectre_info(struct spectre_info *info)
{
	memset(info, 0, sizeof *info);

	if (cpuid_has_feature("smep"))
		info->smep = SMEP_ENABLED;

	if (cpuid_has_feature("ibpb")) {
		info->ibpb = 1;
	}

	if (cpuid_has_feature("ibrs") || cpuid_has_feature("always_ibrs")) {
		info->ibrs = 1;
	}

	if (cpuid_has_feature("stibp") || cpuid_has_feature("always_stibp")) {
		info->stibp = 1;
	}
}
