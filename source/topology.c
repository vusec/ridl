#include <stdlib.h>
#include <string.h>

#include <info/topology.h>

int
check_topology_smt(struct cpu_topology *topo)
{
	struct cpu *cpu;
	size_t i;

	for (i = 0; i < topo->ncpus; ++i) {
		cpu = topo->cpus + i;

		if (bitmap_count(&cpu->thread_siblings) > 1)
			return 1;
	}

	return 0;
}
