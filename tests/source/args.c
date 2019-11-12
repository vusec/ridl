#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <getopt.h>

#include <args.h>
#include <memkit.h>

static char *secret = "Hello world - can't stop the signal!";

int parse_args(struct args *args, int argc, char *argv[])
{
	static struct option long_options[] = {
		{ "rounds", required_argument, 0, 'r' },
		{ "secret", required_argument, 0, 's' },
		{ "threshold", required_argument, 0, 't' },
		{ "write-back", no_argument, 0, ARG_WB },
		{ "write-through", no_argument, 0, ARG_WT },
		{ "write-combine", no_argument, 0, ARG_WC },
		{ "uncacheable-minus", no_argument, 0, ARG_UC_MINUS },
		{ "uncacheable", no_argument, 0, ARG_UC },
		{ "wb", no_argument, 0, ARG_WB },
		{ "wt", no_argument, 0, ARG_WT },
		{ "wc", no_argument, 0, ARG_WC },
		{ "uc-minus", no_argument, 0, ARG_UC_MINUS },
		{ "uc", no_argument, 0, ARG_UC },
		{ "null", no_argument, 0, ARG_NULL },
		{ "demand", no_argument, 0, ARG_DEMAND },
		{ "demand-paging", no_argument, 0, ARG_DEMAND },
		{ "nc", no_argument, 0, ARG_NON_CANON },
		{ "non-canonical", no_argument, 0, ARG_NON_CANON },
		{ 0, 0, 0, 0 },
	};
	int ret = 1;
	int c;
	int opt_idx;

	args->secret = secret;
	args->rounds = 100;
	args->threshold = 100;
	args->leak_src = LEAK_NULL;
	args->mem_type = MEM_WB;

	while (1) {
		opt_idx = 0;
		c = getopt_long(argc, argv, "r:s:", long_options, &opt_idx);

		if (c == -1)
			break;

		switch (c) {
		case 0: break;
		case 'r':
			args->rounds = (size_t)strtoull(optarg, NULL, 0);
			break;
		case 's':
			args->secret = optarg;
			break;
		case 't':
			args->threshold = (uint64_t)strtoull(optarg, NULL, 0);
			break;
		case ARG_WB:
			args->mem_type = MEM_WB;
			break;
		case ARG_WT:
			args->mem_type = MEM_WT;
			break;
		case ARG_WC:
			args->mem_type = MEM_WC;
			break;
		case ARG_UC_MINUS:
			args->mem_type = MEM_UC_MINUS;
			break;
		case ARG_UC:
			args->mem_type = MEM_UC;
			break;
		case ARG_NULL:
			args->leak_src = LEAK_NULL;
			break;
		case ARG_DEMAND:
			args->leak_src = LEAK_DEMAND;
			break;
		case ARG_NON_CANON:
			args->leak_src = LEAK_NON_CANON;
			break;
		default:
			return -1;
		}
	}

	while (optind < argc) {
		argv[ret] = argv[optind];
		++ret;
		++optind;
	}

	return ret;
}

