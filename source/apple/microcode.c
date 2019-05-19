#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/sysctl.h>

char *
get_microcode(void)
{
	char *s = NULL;
	uint64_t value;
	size_t size = sizeof value;

	if (sysctlbyname("machdep.cpu.microcode_version", &value, &size, NULL, 0) < 0)
		return NULL;

	if (asprintf(&s, "0x%llx", value) < 0)
		return NULL;

	return s;
}
