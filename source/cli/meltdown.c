#include <stdio.h>

#include <vuln/meltdown.h>

#include "colors.h"

void
show_meltdown_info(struct meltdown_info *info)
{
	printf(CLI_BOLD "Meltdown:\n" CLI_RESET);

	printf(" * Status: ");

	if (info->affected) {
		printf(CLI_BOLD CLI_RED "Vulnerable\n" CLI_RESET);
	} else {
		printf(CLI_BOLD CLI_GREEN "Not Affected\n" CLI_RESET);
	}

	printf(" * KPTI Present: ");

	if (info->kpti_present) {
		printf(CLI_BOLD CLI_GREEN "Yes\n" CLI_RESET);
	} else {
		printf(CLI_BOLD CLI_RED "No\n" CLI_RESET);
	}

	printf(" * KPTI Enabled: ");

	if (info->kpti_enabled) {
		printf(CLI_BOLD CLI_GREEN "Yes\n" CLI_RESET);
	} else if (info->affected) {
		printf(CLI_BOLD CLI_RED "No\n" CLI_RESET);
	} else {
		printf(CLI_BOLD "No\n" CLI_RESET);
	}

	printf(" * PCID Accelerated: ");

	if (info->has_pcid) {
		printf(CLI_BOLD "Yes\n" CLI_RESET);
	} else {
		printf(CLI_BOLD "No\n" CLI_RESET);
	}

	printf(" * PCID Invalidation: ");

	if (info->has_invpcid) {
		printf(CLI_BOLD "Yes\n" CLI_RESET);
	} else {
		printf(CLI_BOLD "No\n" CLI_RESET);
	}
}
