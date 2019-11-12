#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <vuln/meltdown.h>

#include <info/cpuid.h>

#include "vuln.h"

void
query_meltdown_info(struct meltdown_info *info)
{
	struct vuln_iter iter;

	memset(info, 0, sizeof *info);

	info->has_pcid = cpuid_has_feature("pcid");
	info->has_invpcid = cpuid_has_feature("invpcid");

	if (iter_vuln(&iter, "meltdown") < 0)
		return;

	info->kpti_present = 1;

	while (next_vuln(&iter) == 0) {
		if (strcmp(iter.key, "Vulnerable") == 0) {
			info->affected = 1;
		} else if (strcmp(iter.key, "Not affected") == 0) {
			info->affected = 0;
		} else if (iter.value && strcmp(iter.value, "PTI") == 0) {
			info->affected = 1;
			info->kpti_enabled = 1;
		}
	}
}
