#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <info/cpuid.h>

#include <vuln/ssb.h>

void
query_ssb_info(struct ssb_info *info)
{
	memset(info, 0, sizeof *info);

	info->affected = 1;

	if (cpuid_has_feature("ssbd"))
		info->ssbd = SSBD_PRESENT;

	if (cpuid_has_feature("no_ssb"))
		info->affected = 0;
}
