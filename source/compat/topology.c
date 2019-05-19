#include <stdlib.h>
#include <string.h>

#include <info/topology.h>

size_t
get_thread_count(void)
{
	struct cpu_topology topo;
	size_t count;

	if (read_cpu_topology(&topo) < 0)
		return 1;

	count = topo.ncpus;

	free_cpu_topology(&topo);

	return count;
}

size_t
get_core_count(void)
{
	struct bitmap mask;
	struct cpu_topology topo;
	struct cpu *cpu;
	size_t count = 1;
	size_t i;

	if (read_cpu_topology(&topo) < 0)
		return 1;

	if (bitmap_alloc(&mask, topo.ncpus) < 0)
		goto err_free_topology;

	count = 0;

	for (i = 0; i < topo.ncpus; ++i) {
		cpu = topo.cpus + i;

		if (bitmap_is_set(&mask, i))
			continue;

		bitmap_or(&mask, &cpu->thread_siblings);
		++count;
	}

	bitmap_free(&mask);
	free_cpu_topology(&topo);

	return count;

err_free_topology:
	free_cpu_topology(&topo);
	return count;
}
