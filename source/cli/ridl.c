#include <stdio.h>

#include <vuln/ridl.h>

#include "colors.h"

void
show_ridl_info(struct ridl_info *info)
{
	printf(CLI_BOLD "Micro-architectural Data Sampling:\n" CLI_RESET);
	printf(" * Line Fill Buffers (MFBDS): ");

	if (info->mfbds) {
		printf(CLI_BOLD CLI_RED "Vulnerable\n" CLI_RESET);
	} else {
		printf(CLI_BOLD CLI_GREEN "Not Affected\n" CLI_RESET);
	}

	printf(" * Store Buffers (MSBDS): ");

	if (info->msbds) {
		printf(CLI_BOLD CLI_RED "Vulnerable\n" CLI_RESET);
	} else {
		printf(CLI_BOLD CLI_GREEN "Not Affected\n" CLI_RESET);
	}

	printf(" * Load Ports (MLPDS): ");

	if (info->msbds) {
		printf(CLI_BOLD CLI_RED "Vulnerable\n" CLI_RESET);
	} else {
		printf(CLI_BOLD CLI_GREEN "Not Affected\n" CLI_RESET);
	}

	printf(" * Uncached Memory (MDSUM): ");

	if (info->msbds) {
		printf(CLI_BOLD CLI_RED "Vulnerable\n" CLI_RESET);
	} else {
		printf(CLI_BOLD CLI_GREEN "Not Affected\n" CLI_RESET);
	}

	printf(" * SMT: ");

	if ((info->mfbds || info->msbds || info->mlpds) && info->smt_vuln) {
		printf(CLI_BOLD CLI_RED "Vulnerable\n" CLI_RESET);
	} else if (info->mfbds || info->msbds || info->mlpds) {
		printf(CLI_BOLD CLI_GREEN "Unaffected\n" CLI_RESET);
	} else {
		printf(CLI_BOLD "Unaffected\n" CLI_RESET);
	}

	printf(" * MD_CLEAR: ");

	if (info->mfbds || info->msbds || info->mlpds) {
		if (info->md_clear) {
			printf(CLI_BOLD CLI_GREEN "Available\n" CLI_RESET);
		} else {
			printf(CLI_BOLD CLI_RED "Not Available\n" CLI_RESET);
		}
	} else {
		printf(CLI_BOLD CLI_GREEN "Not Required\n" CLI_RESET);
	}
}
