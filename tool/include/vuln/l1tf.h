#pragma once

enum {
	L1D_FLUSH_NEVER = 0,
	L1D_FLUSH_AVAIL,
	L1D_FLUSH_COND,
	L1D_FLUSH_ALWAYS,
};

struct l1tf_info {
	int affected;
	int l1tf_present;
	int pte_inv;
	int smt_vuln;
	int l1d_flush;
	int has_l1d_flush;
};

void
query_l1tf_info(struct l1tf_info *info);
