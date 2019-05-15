#include <stdio.h>

#include <vuln/l1tf.h>

#include "colors.h"

void
show_l1tf_info(struct l1tf_info *info)
{
	printf(CLI_BOLD "L1 Terminal Fault:\n" CLI_RESET);

	printf(" * Status: ");

	if (info->affected) {
		printf(CLI_BOLD CLI_RED "Vulnerable\n" CLI_RESET);
	} else {
		printf(CLI_BOLD CLI_GREEN "Not Affected\n" CLI_RESET);
	}

	printf(" * L1TF Present: ");

	if (info->l1tf_present) {
		printf(CLI_BOLD CLI_GREEN "Yes\n" CLI_RESET);
	} else if (info->affected) {
		printf(CLI_BOLD CLI_RED "No\n" CLI_RESET);
	} else {
		printf(CLI_BOLD "No\n" CLI_RESET);
	}

	printf(" * PTE Inversion: ");

	if (info->pte_inv) {
		printf(CLI_BOLD CLI_GREEN "Yes\n" CLI_RESET);
	} else if (info->affected) {
		printf(CLI_BOLD CLI_RED "No\n" CLI_RESET);
	} else {
		printf(CLI_BOLD "No\n" CLI_RESET);
	}

	printf(" * SMT: ");

	if (info->affected && info->smt_vuln) {
		printf(CLI_BOLD CLI_RED "Vulnerable\n" CLI_RESET);
	} else if (info->affected) {
		printf(CLI_BOLD CLI_GREEN "Unaffected\n" CLI_RESET);
	} else {
		printf(CLI_BOLD "Unaffected\n" CLI_RESET);
	}


	printf(" * L1d Flush Present: ");

	if (info->has_l1d_flush) {
		printf(CLI_BOLD "Yes\n" CLI_RESET);
	} else {
		printf(CLI_BOLD "No\n" CLI_RESET);
	}

	printf(" * L1d Flush: ");

	switch (info->l1d_flush) {
	case L1D_FLUSH_ALWAYS:
		printf(CLI_BOLD CLI_GREEN "Always\n" CLI_RESET);
		break;
	case L1D_FLUSH_COND:
		printf(CLI_BOLD CLI_GREEN "Conditional\n" CLI_RESET);
		break;
	case L1D_FLUSH_AVAIL:
		printf(CLI_BOLD CLI_YELLOW "Available\n" CLI_RESET);
		break;
	default:
		if (info->affected)
			printf(CLI_BOLD CLI_RED "Never\n" CLI_RESET);
		else
			printf(CLI_BOLD "Never\n" CLI_RESET);
		break;
	}
}
