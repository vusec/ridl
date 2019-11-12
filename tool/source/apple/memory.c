#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/sysctl.h>

#include <human.h>

char *
get_memory_size(int binary, size_t prec)
{
	char *s = NULL;
	uint64_t value;
	size_t size = sizeof value;

	if (sysctlbyname("hw.memsize", &value, &size, NULL, 0) < 0)
		return NULL;

	return human_file_size((double)value, binary, prec); 
}
