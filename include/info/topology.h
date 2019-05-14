#pragma once

#include <bitmap.h>

struct cpu {
	struct bitmap thread_siblings;
	struct bitmap core_siblings;
	unsigned long package_id;
	unsigned long core_id;
};

struct cpu_topology {
	struct cpu *cpus;
	size_t ncpus;
};

int
check_topology_smt(struct cpu_topology *topo);
int
read_cpu_topology(struct cpu_topology *topo);
