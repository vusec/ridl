#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <vuln/meltdown.h>

#define WIN32_MEAN_AND_LEAN
#include <ntstatus.h>
#define WIN32_NO_STATUS
#include <windows.h>
#include <winternl.h>

#include "ntex.h"

void
query_meltdown_info(struct meltdown_info *info)
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
		info->kpti_present = 1;
		return;
	}

	if (!NT_SUCCESS(status)) {
		return;
	}

	info->kpti_present = 1;
	info->kpti_enabled = kva_info.KvaShadowFlags.KvaShadowEnabled;
	info->affected = kva_info.KvaShadowFlags.KvaShadowRequired;
	info->has_pcid = kva_info.KvaShadowFlags.KvaShadowPcid;
	info->has_invpcid = kva_info.KvaShadowFlags.KvaShadowInvpcid;
}
