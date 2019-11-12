#pragma once

enum leak_src {
	LEAK_NULL,
	LEAK_DEMAND,
	LEAK_NON_CANON,
};

struct args {
	char *secret;
	size_t rounds;
	uint64_t threshold;
	unsigned mem_type;
	enum leak_src leak_src;
};

enum arg {
	ARG_WB = 256,
	ARG_WT,
	ARG_WC,
	ARG_UC_MINUS,
	ARG_UC,
	ARG_NULL,
	ARG_DEMAND,
	ARG_NON_CANON,
};

int parse_args(struct args *args, int argc, char *argv[]);

