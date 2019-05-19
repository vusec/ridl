#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <info/cpuid.h>
#include <info/topology.h>

#include <vuln/l1tf.h>

void
cpu_query_l1tf_info(struct l1tf_info *info);

void
query_l1tf_info(struct l1tf_info *info)
{
	memset(info, 0, sizeof *info);

	cpu_query_l1tf_info(info);

	if (check_smt()) {
		info->smt_vuln = 1;
	}

	if (cpuid_has_feature("l1d_flush")) {
		info->l1d_flush = L1D_FLUSH_AVAIL;
	}
}
