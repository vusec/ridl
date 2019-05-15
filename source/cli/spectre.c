#include <stdio.h>

#include <vuln/spectre.h>

#include "colors.h"

void
show_spectre_v1_info(struct spectre_info *info)
{
	printf(CLI_BOLD "Direct Branch Speculation:\n" CLI_RESET);

	printf(" * Status: ");

	if (info->v1_affected) {
		printf(CLI_BOLD CLI_RED "Vulnerable\n" CLI_RESET);
	} else {
		printf(CLI_BOLD CLI_GREEN "Not Affected\n" CLI_RESET);
	}

	printf(" * __user pointer sanitization: ");

	if (info->uptr_san) {
		printf(CLI_BOLD CLI_GREEN "Enabled\n" CLI_RESET);
	} else if (info->v1_affected) {
		printf(CLI_BOLD CLI_RED "Disabled\n" CLI_RESET);
	} else {
		printf(CLI_BOLD "Disabled\n" CLI_RESET);
	}
}

void
show_spectre_v2_info(struct spectre_info *info)
{
	printf(CLI_BOLD "Indirect Branch Speculation:\n" CLI_RESET);

	printf(" * Status: ");

	if (info->v2_affected) {
		printf(CLI_BOLD CLI_RED "Vulnerable\n" CLI_RESET);
	} else {
		printf(CLI_BOLD CLI_GREEN "Not Affected\n" CLI_RESET);
	}

	printf(" * Retpoline: ");

	switch (info->retpol) {
	case RETPOL_ASM:
		printf(CLI_BOLD CLI_GREEN "Assembly\n" CLI_RESET);
		break;
	case RETPOL_FULL:
		printf(CLI_BOLD CLI_GREEN "Full\n" CLI_RESET);
		break;
	default:
		if (info->v2_affected)
			printf(CLI_BOLD CLI_RED "Disabled\n" CLI_RESET);
		else
			printf(CLI_BOLD "Disabled\n" CLI_RESET);
	}

	printf(" * IBPB: ");

	switch (info->ibpb) {
	case IBPB_ALWAYS:
		printf(CLI_BOLD CLI_GREEN "Always\n" CLI_RESET);
		break;
	case IBPB_COND:
		printf(CLI_BOLD CLI_GREEN "Conditional\n" CLI_RESET);
		break;
	case IBPB_PRESENT:
		if (info->v2_affected)
			printf(CLI_BOLD CLI_RED "Disabled\n" CLI_RESET);
		else
			printf(CLI_BOLD "Disabled\n" CLI_RESET);
		break;
	default:
		if (info->v2_affected)
			printf(CLI_BOLD CLI_RED "Not Available\n" CLI_RESET);
		else
			printf(CLI_BOLD "Not Available\n" CLI_RESET);
	}

	printf(" * IBRS: ");

	switch (info->ibrs) {
	case IBRS_ENABLED:
		printf(CLI_BOLD CLI_GREEN "Enabled\n" CLI_RESET);
		break;
	case IBRS_PRESENT:
		if (info->v2_affected)
			printf(CLI_BOLD CLI_RED "Disabled\n" CLI_RESET);
		else
			printf(CLI_BOLD "Disabled\n" CLI_RESET);
		break;
	default:
		if (info->v2_affected)
			printf(CLI_BOLD CLI_RED "Not Available\n" CLI_RESET);
		else
			printf(CLI_BOLD "Not Available\n" CLI_RESET);
	}

	printf(" * STIBP: ");

	switch (info->stibp) {
	case STIBP_ENABLED:
		printf(CLI_BOLD CLI_GREEN "Enabled\n" CLI_RESET);
		break;
	case STIBP_PRESENT:
		if (info->v2_affected)
			printf(CLI_BOLD CLI_RED "Disabled\n" CLI_RESET);
		else
			printf(CLI_BOLD "Disabled\n" CLI_RESET);
		break;
	default:
		if (info->v2_affected)
			printf(CLI_BOLD CLI_RED "Not Available\n" CLI_RESET);
		else
			printf(CLI_BOLD "Not Available\n" CLI_RESET);
	}

	printf(" * SMEP: ");

	switch (info->smep) {
	case SMEP_ENABLED:
		printf(CLI_BOLD CLI_GREEN "Enabled\n" CLI_RESET);
		break;
	case SMEP_PRESENT:
		if (info->v2_affected)
			printf(CLI_BOLD CLI_RED "Disabled\n" CLI_RESET);
		else
			printf(CLI_BOLD "Disabled\n" CLI_RESET);
	default:
		if (info->v2_affected)
			printf(CLI_BOLD CLI_RED "Not Available\n" CLI_RESET);
		else
			printf(CLI_BOLD "Not Available\n" CLI_RESET);
	}
}
