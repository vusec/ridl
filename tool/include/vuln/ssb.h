#pragma once

enum {
	SSBD_NONE = 0,
	SSBD_PRESENT,
	SSBD_OS_SUPPORT,
};

struct ssb_info {
	int affected;
	int ssbd;
};

void
query_ssb_info(struct ssb_info *info);
