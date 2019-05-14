#pragma once

struct meltdown_info {
	int affected;
	int kpti_present;
	int kpti_enabled;
	int has_pcid;
	int has_invpcid;
};

void
query_meltdown_info(struct meltdown_info *info);
