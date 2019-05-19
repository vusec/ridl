#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <info/cpuid.h>

#include <vuln/ssb.h>

#include "os.h"

void
query_ssb_info(struct ssb_info *info)
{
	int major, minor, patch;

	memset(info, 0, sizeof *info);

	info->affected = 1;

	if (cpuid_has_feature("ssbd"))
		info->ssbd = SSBD_PRESENT;

	if (cpuid_has_feature("no_ssb"))
		info->affected = 0;

	major = get_os_version(VERSION_MAJOR);
	minor = get_os_version(VERSION_MINOR);
	patch = get_os_version(VERSION_PATCH);

	if (major == 10 &&
		((minor == 13 && patch >= 6) ||
		  minor >= 14)) {
		info->ssbd = SSBD_OS_SUPPORT;
	}
}
