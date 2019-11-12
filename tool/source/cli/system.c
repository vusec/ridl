#include <stdio.h>

#include <info/cpuid.h>

#include "colors.h"

#include "../system.h"

void
show_system_info(struct sys_info *info)
{
	printf(CLI_BOLD "System:\n" CLI_RESET);

	printf(" * Operating System: " CLI_BOLD "%s\n" CLI_RESET,
		info->os_name);
	printf(" * Processor: " CLI_BOLD "%s\n" CLI_RESET,
		info->cpu_name);
	printf(" * Microarchitecture: " CLI_BOLD "%s\n" CLI_RESET,
		cpuid_get_codename());
	printf(" * Microcode: " CLI_BOLD "%s\n" CLI_RESET,
		info->microcode);
	printf(" * Memory: " CLI_BOLD "%s\n" CLI_RESET,
		info->memory);
}
