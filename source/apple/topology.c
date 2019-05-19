#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <asprintf.h>
#include <bitmap.h>
#include <macros.h>

#include <info/topology.h>

int
read_cpu_topology(struct cpu_topology *topo)
{
	return -1;
}

size_t
get_thread_count(void)
{
	uint64_t value;
	size_t size = sizeof value;

	if (sysctlbyname("hw.logicalcpu", &value, &size, NULL, 0) < 0)
		return 1;

	return value;
}

size_t
get_core_count(void)
{
	uint64_t value;
	size_t size = sizeof value;

	if (sysctlbyname("hw.physicalcpu", &value, &size, NULL, 0) < 0)
		return 1;

	return value;
}
