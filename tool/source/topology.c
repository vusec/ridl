#include <stdlib.h>
#include <string.h>

#include <info/topology.h>

void
free_cpu_topology(struct cpu_topology *topo)
{
	struct cpu *cpu;
	size_t i;

	if (!topo)
		return;

	for (i = 0; i < topo->ncpus; ++i) {
		cpu = topo->cpus + i;

		bitmap_free(&cpu->thread_siblings);
		bitmap_free(&cpu->core_siblings);
	}

	if (topo->cpus) {
		free(topo->cpus);
	}

	topo->cpus = NULL;
	topo->ncpus = 0;
}

int
check_smt(void)
{
	return get_thread_count() > get_core_count();
}
