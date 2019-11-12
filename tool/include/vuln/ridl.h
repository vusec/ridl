#pragma once

struct ridl_info {
	int mfbds, msbds, mlpds, mdsum;
	int md_clear;
	int smt_vuln;
};

void
query_ridl_info(struct ridl_info *info);
