#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/sysctl.h>

char *
get_microcode(void)
{
	char *value;
	size_t size;

	if (sysctlbyname("machdep.cpu.microcode_value", NULL, &size, NULL, 0) < 0)
		return NULL;

	value = malloc(size);

	if (!value)
		return NULL;

	if (sysctlbyname("machdep.cpu.microcode_value", &value, &size, NULL, 0) < 0)
		return NULL;

	return value;
}
