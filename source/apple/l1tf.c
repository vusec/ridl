#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <info/cpuid.h>
#include <info/topology.h>

#include <vuln/l1tf.h>

void
query_l1tf_info(struct l1tf_info *info)
{
	memset(info, 0, sizeof *info);

	if (cpuid_has_feature("l1d_flush")) {
		info->l1d_flush = L1D_FLUSH_AVAIL;
	}
}
