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
read_cpu_topology(struct cpu_topology *topo);
void
free_cpu_topology(struct cpu_topology *topo);
size_t
get_thread_count(void);
size_t
get_core_count(void);
int
check_smt(void);
