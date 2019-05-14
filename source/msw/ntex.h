#pragma once

#define SystemSpeculationControlInformation (SYSTEM_INFORMATION_CLASS)201
#define SystemKernelVaShadowInformation (SYSTEM_INFORMATION_CLASS)196

typedef struct _SYSTEM_SPECULATION_CONTROL_INFORMATION {
	struct {
		ULONG BpbEnabled : 1;
		ULONG BpbDisabledSystemPolicy : 1;
		ULONG BpbDisabledNoHardwareSupport : 1;
		ULONG SpecCtrlEnumerated : 1;
		ULONG SpecCmdEnumerated : 1;
		ULONG IbrsPresent : 1;
		ULONG StibpPresent : 1;
		ULONG SmepPresent : 1;
		ULONG SpeculativeStoreBypassDisableAvailable : 1;
		ULONG SpeculativeStoreBypassDisableSupported : 1;
		ULONG SpeculativeStoreBypassDisabledSystemWide : 1;
		ULONG SpeculativeStoreBypassDisabledKernel : 1;
		ULONG SpeculativeStoreBypassDisableRequired : 1;
		ULONG BpbDisabledKernelToUser : 1;
		ULONG SpecCtrlRetpolineEnabled : 1;
		ULONG SpecCtrlImportOptimizationEnabled : 1;
		ULONG Reserved : 16;
	} SpeculationControlFlags;
} SYSTEM_SPECULATION_CONTROL_INFORMATION, * PSYSTEM_SPECULATION_CONTROL_INFORMATION;

typedef struct _SYSTEM_KERNEL_VA_SHADOW_INFORMATION {
	struct {
		ULONG KvaShadowEnabled : 1;
		ULONG KvaShadowUserGlobal : 1;
		ULONG KvaShadowPcid : 1;
		ULONG KvaShadowInvpcid : 1;
		ULONG KvaShadowRequired : 1;
		ULONG KvaShadowRequiredAvailable : 1;
		ULONG InvalidPteBit : 6;
		ULONG L1DataCacheFlushSupported : 1;
		ULONG L1TerminalFaultMitigationPresent : 1;
		ULONG Reserved : 18;
	} KvaShadowFlags;
} SYSTEM_KERNEL_VA_SHADOW_INFORMATION, * PSYSTEM_KERNEL_VA_SHADOW_INFORMATION;
