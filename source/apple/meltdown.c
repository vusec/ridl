#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <vuln/meltdown.h>

#include <info/cpuid.h>

void
cpu_query_meltdown_info(struct meltdown_info *info);

void
query_meltdown_info(struct meltdown_info *info)
{
	memset(info, 0, sizeof *info);

	cpu_query_meltdown_info(info);

	info->has_pcid = cpuid_has_feature("pcid");
	info->has_invpcid = cpuid_has_feature("invpcid");
}
