#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <vuln/spectre.h>

#define WIN32_MEAN_AND_LEAN
#include <ntstatus.h>
#define WIN32_NO_STATUS
#include <windows.h>
#include <winternl.h>

#include "ntex.h"

void
query_spectre_info(struct spectre_info *info)
{
	SYSTEM_SPECULATION_CONTROL_INFORMATION spec_info;
	NTSTATUS status;

	memset(info, 0, sizeof *info);

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

	info->v1_affected = 1;
	info->v2_affected = 1;
	info->uptr_san = 0;

	if (spec_info.SpeculationControlFlags.SpecCtrlRetpolineEnabled) {
		info->retpol = RETPOL_FULL;
	}

	if (spec_info.SpeculationControlFlags.BpbEnabled) {
		info->ibpb = IBPB_ALWAYS;
	} else if (spec_info.SpeculationControlFlags.SpecCmdEnumerated) {
		info->ibpb = IBPB_PRESENT;
	}

	if (spec_info.SpeculationControlFlags.IbrsPresent) {
		info->ibrs = IBRS_ENABLED;
	} else if (spec_info.SpeculationControlFlags.SpecCtrlEnumerated) {
		info->ibrs = IBRS_PRESENT;
	}

	if (spec_info.SpeculationControlFlags.StibpPresent) {
		info->stibp = STIBP_ENABLED;
	} else if (spec_info.SpeculationControlFlags.SpecCtrlEnumerated) {
		info->stibp = STIBP_PRESENT;
	}

	info->smep = spec_info.SpeculationControlFlags.SmepPresent;
}
