#pragma once

enum {
	RETPOL_NONE = 0,
	RETPOL_ASM,
	RETPOL_FULL,
};

enum {
	IBPB_NONE = 0,
	IBPB_PRESENT,
	IBPB_COND,
	IBPB_ALWAYS,
};

enum {
	IBRS_NONE = 0,
	IBRS_PRESENT,
	IBRS_ENABLED,
};

enum {
	STIBP_NONE = 0,
	STIBP_PRESENT,
	STIBP_ENABLED,
};

enum {
	SMEP_NONE = 0,
	SMEP_PRESENT,
	SMEP_ENABLED,
};

struct spectre_info {
	int v1_affected;
	int v2_affected;
	int uptr_san;
	int retpol;
	int ibpb, ibrs, stibp;
	int rsb_fill;
	int smep;
};

void
query_spectre_info(struct spectre_info *info);
