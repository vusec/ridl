#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <vuln/ssb.h>

#define WIN32_MEAN_AND_LEAN
#include <ntstatus.h>
#define WIN32_NO_STATUS
#include <windows.h>
#include <winternl.h>

#include "ntex.h"

void
query_ssb_info(struct ssb_info *info)
{
	SYSTEM_SPECULATION_CONTROL_INFORMATION spec_info;
	NTSTATUS status;

	memset(info, 0, sizeof *info);

	info->affected = 1;

	status = NtQuerySystemInformation(SystemSpeculationControlInformation,
		&spec_info, sizeof spec_info, NULL);

	if (status == STATUS_INVALID_INFO_CLASS) {
		return;
	}

	if (status == STATUS_NOT_IMPLEMENTED) {
		return;
	}

	if (!NT_SUCCESS(status)) {
		return;
	}

	if (!spec_info.SpeculationControlFlags.SpeculativeStoreBypassDisableRequired) {
		info->affected = 0;
	}

	if (spec_info.SpeculationControlFlags.SpeculativeStoreBypassDisableAvailable) {
		info->ssbd = SSBD_PRESENT;
	}

	if (spec_info.SpeculationControlFlags.SpeculativeStoreBypassDisableSupported) {
		info->ssbd = SSBD_OS_SUPPORT;
	}
}
