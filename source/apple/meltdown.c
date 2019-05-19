#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <vuln/meltdown.h>

#include <info/cpuid.h>

#include "os.h"

void
cpu_query_meltdown_info(struct meltdown_info *info);

void
query_meltdown_info(struct meltdown_info *info)
{
	int major, minor, patch;

	memset(info, 0, sizeof *info);

	cpu_query_meltdown_info(info);

	info->has_pcid = cpuid_has_feature("pcid");
	info->has_invpcid = cpuid_has_feature("invpcid");

	major = get_os_version(VERSION_MAJOR);
	minor = get_os_version(VERSION_MINOR);
	patch = get_os_version(VERSION_PATCH);

	if (major == 10 &&
		((minor == 11 && patch >= 6) ||
		 (minor == 12 && patch >= 6) ||
		 (minor == 13 && patch >= 2) ||
		  minor >= 14)) {
		info->kpti_present = 1;
		info->kpti_enabled = info->affected;
	}
}
