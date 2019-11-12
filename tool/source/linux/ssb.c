#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <info/cpuid.h>

#include <vuln/ssb.h>

#include "vuln.h"

void
query_ssb_info(struct ssb_info *info)
{
	struct vuln_iter iter;

	memset(info, 0, sizeof *info);

	info->affected = 1;

	if (cpuid_has_feature("ssbd"))
		info->ssbd = SSBD_PRESENT;

	if (cpuid_has_feature("no_ssb"))
		info->affected = 0;

	if (iter_vuln(&iter, "spec_store_bypass") < 0)
		return;

	while (next_vuln(&iter) == 0) {
		if (strcmp(iter.key, "Vulnerable") == 0) {
			info->affected = 1;
		} else if (strcmp(iter.key, "Not affected") == 0) {
			info->affected = 0;
		} else if (strcmp(iter.key, "Mitigation") == 0 &&
			strcmp(iter.value, "Speculative Store Bypass disabled via prctl and seccomp") == 0) {
			info->ssbd = SSBD_OS_SUPPORT;
		}
	}
}
