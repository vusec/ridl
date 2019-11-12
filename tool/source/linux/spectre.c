#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <info/cpuid.h>

#include <vuln/spectre.h>

#include "vuln.h"

void
query_spectre_info(struct spectre_info *info)
{
	struct vuln_iter iter;

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

	if (iter_vuln(&iter, "spectre_v1") < 0)
		return;

	while (next_vuln(&iter) == 0) {
		if (strcmp(iter.key, "Vulnerable") == 0) {
			info->v1_affected = 1;
		} else if (strcmp(iter.key, "Not affected") == 0) {
			info->v1_affected = 0;
		} else if (strcmp(iter.key, "Mitigation") == 0 &&
			strcmp(iter.value, "__user pointer sanitization") == 0) {
			info->v1_affected = 1;
			info->uptr_san = 1;
		}
	}

	if (iter_vuln(&iter, "spectre_v2") < 0)
		return;

	while (next_vuln(&iter) == 0) {
		if (strcmp(iter.key, "Vulnerable") == 0) {
			info->v2_affected = 1;
		} else if (strcmp(iter.key, "Not affected") == 0) {
			info->v2_affected = 0;
		} else if (iter.value && strcmp(iter.value, "Full AMD retpoline") == 0) {
			info->v2_affected = 1;
			info->retpol = RETPOL_FULL;
		} else if (iter.value && strcmp(iter.value, "Full generic retpoline") == 0) {
			info->v2_affected = 1;
			info->retpol = RETPOL_FULL;
		} else if (iter.value && strcmp(iter.value, "IBPB") == 0) {
			info->v2_affected = 1;
			info->ibpb = IBPB_ALWAYS;
		} else if (strcmp(iter.key, "IBPB") == 0 && iter.value &&
			strcmp(iter.value, "conditional") == 0) {
			info->v2_affected = 1;
			info->ibpb = IBPB_COND;
		} else if (iter.value && strcmp(iter.value, "IBRS") == 0) {
			info->v2_affected = 1;
			info->ibrs = IBRS_ENABLED;
		} else if (iter.value && strcmp(iter.value, "IBRS_FW") == 0) {
			info->v2_affected = 1;
			info->ibrs = IBRS_ENABLED;
		} else if (strcmp(iter.key, "RSB filling") == 0) {
			info->v2_affected = 1;
			info->rsb_fill = 1;
		}
	}
}
