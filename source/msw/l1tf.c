#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <vuln/l1tf.h>

#define WIN32_MEAN_AND_LEAN
#include <ntstatus.h>
#define WIN32_NO_STATUS
#include <windows.h>
#include <winternl.h>

#include "ntex.h"

void
query_l1tf_info(struct l1tf_info *info)
{
	SYSTEM_KERNEL_VA_SHADOW_INFORMATION kva_info;
	NTSTATUS status;

	memset(info, 0, sizeof *info);

	status = NtQuerySystemInformation(SystemKernelVaShadowInformation,
		&kva_info, sizeof kva_info, NULL);

	if (status == STATUS_INVALID_INFO_CLASS) {
		return;
	}

	if (status == STATUS_NOT_IMPLEMENTED) {
		info->l1tf_present = 1;
		return;
	}

	if (!NT_SUCCESS(status)) {
		return;
	}

	info->l1tf_present = 1;
	info->pte_inv = kva_info.KvaShadowFlags.InvalidPteBit;
	info->affected = kva_info.KvaShadowFlags.KvaShadowRequired;
	info->smt_vuln = 1; /* TODO: how do we know? */
	info->l1d_flush = kva_info.KvaShadowFlags.L1DataCacheFlushSupported;
}
