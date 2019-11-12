#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <info/cpuid.h>
#include <info/topology.h>

#include <vuln/l1tf.h>

#include "vuln.h"

void
query_l1tf_info(struct l1tf_info *info)
{
	struct cpu_topology topo;
	struct vuln_iter iter;

	memset(info, 0, sizeof *info);

	if (cpuid_has_feature("l1d_flush")) {
		info->l1d_flush = L1D_FLUSH_AVAIL;
	}

	if (check_smt()) {
		info->smt_vuln = 1;
	}

	if (iter_vuln(&iter, "l1tf") < 0)
		return;

	info->l1tf_present = 1;

	while (next_vuln(&iter) == 0) {
		if (strcmp(iter.key, "Not affected") == 0) {
			info->affected = 0;
		} else if (strcmp(iter.key, "Mitigation") == 0 &&
			strcmp(iter.value, "PTE Inversion") == 0) {
			info->affected = 1;
			info->pte_inv = 1;
		} else if (iter.value && strcmp(iter.value, "SMT vulnerable") == 0) {
			info->affected = 1;
			info->smt_vuln = 1;
		} else if (iter.value && strcmp(iter.value, "conditional cache flushes") == 0) {
			info->affected = 1;
			info->l1d_flush = L1D_FLUSH_COND;
		} else if (iter.value && strcmp(iter.value, "cache flushes") == 0) {
			info->affected = 1;
			info->l1d_flush = L1D_FLUSH_ALWAYS;
		}
	}
}
