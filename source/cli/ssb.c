#include <stdio.h>

#include <vuln/ssb.h>

#include "colors.h"

void
show_ssb_info(struct ssb_info *info)
{
	float ratios[] = {0.3f, 0.7f};
	int ret;

	printf(CLI_BOLD "Speculative Store Bypass:\n" CLI_RESET);

	printf(" * Status: ");

	if (info->affected) {
		printf(CLI_BOLD CLI_RED "Vulnerable\n" CLI_RESET);
	} else {
		printf(CLI_BOLD CLI_GREEN "Not Affected\n" CLI_RESET);
	}

	printf(" * Speculative Store Bypass Disable: ");

	if (info->affected) {
		switch (info->ssbd) {
		case SSBD_OS_SUPPORT:
			printf(CLI_BOLD CLI_GREEN "OS Support\n" CLI_RESET);
			break;
		case SSBD_PRESENT:
			printf(CLI_BOLD CLI_YELLOW "Available\n" CLI_RESET);
			break;
		default:
			printf(CLI_BOLD CLI_RED "Not Available\n" CLI_RESET);
			break;
		}
	} else {
		printf(CLI_BOLD CLI_GREEN "Not Required\n" CLI_RESET);
	}
}
