#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <asprintf.h>
#include <bitmap.h>
#include <macros.h>

#include <info/topology.h>

int
sysfs_read_ul(unsigned long *value, const char *path, int base)
{
	FILE *f;
	char *line = NULL;
	size_t n;
	int ret;

	f = fopen(path, "r");

	if (!f)
		return -1;

	ret = getline(&line, &n, f);
	fclose(f);

	if (ret < 0)
		return -1;

	*value = strtoul(line, NULL, base);
	free(line);

	return 0;
}

int
sysfs_parse_max_from_list(unsigned long *value, const char *path)
{
	FILE *f;
	char *line = NULL;
	char *delim;
	size_t n;
	unsigned long best = 0;
	unsigned long parsed;

	f = fopen(path, "r");

	if (!f)
		return -1;

	while (getdelim(&line, &n, ',', f) >= 0) {
		delim = strchr(line, '-');

		if (delim) {
			++delim;
			parsed = strtoul(delim, NULL, 10);
		} else {
			parsed = strtoul(line, NULL, 10);
		}

		best = _MAX(best, parsed);

		free(line);
		line = NULL;
	}

	fclose(f);
	*value = best;

	return 0;
}

int
sysfs_parse_list(struct bitmap *cpu_mask, const char *path)
{
	FILE *f;
	char *line = NULL;
	char *delim;
	size_t n;
	unsigned long from;
	unsigned long to;

	f = fopen(path, "r");

	if (!f)
		return -1;

	while (getdelim(&line, &n, ',', f) >= 0) {
		delim = strchr(line, '-');

		if (delim) {
			*delim++ = '\0';

			from = strtoul(line, NULL, 10);
			to = strtoul(delim, NULL, 10);
		} else {
			from = strtoul(line, NULL, 10);
			to = from;
		}

		while (from <= to) {
			bitmap_set(cpu_mask, from);
			++from;
		}

		free(line);
		line = NULL;
	}

	fclose(f);

	return 0;
}

int
get_thread_siblings(struct bitmap *cpu_mask, size_t cpu_no)
{
	return 0;
}

int
read_core_id(unsigned long *value, unsigned long cpu_no)
{
	char *path = NULL;
	int ret;

	if (asprintf(&path, "/sys/devices/system/cpu/cpu%lu/topology/core_id",
		cpu_no) < 0)
		return -1;

	ret = sysfs_read_ul(value, path, 10);
	free(path);

	return ret;
}

int
read_package_id(unsigned long *value, unsigned long cpu_no)
{
	char *path = NULL;
	int ret;

	if (asprintf(&path, "/sys/devices/system/cpu/cpu%lu/topology/physical_package_id",
		cpu_no) < 0)
		return -1;

	ret = sysfs_read_ul(value, path, 10);
	free(path);

	return ret;
}

int
read_core_siblings(struct bitmap *cpu_mask, unsigned long cpu_no)
{
	char *path = NULL;
	int ret;

	if (asprintf(&path, "/sys/devices/system/cpu/cpu%lu/topology/core_siblings_list",
		cpu_no) < 0)
		return -1;

	ret = sysfs_parse_list(cpu_mask, path);
	free(path);

	return 0;
}

int
read_thread_siblings(struct bitmap *cpu_mask, unsigned long cpu_no)
{
	char *path = NULL;
	int ret;

	if (asprintf(&path, "/sys/devices/system/cpu/cpu%lu/topology/thread_siblings_list",
		cpu_no) < 0)
		return -1;

	ret = sysfs_parse_list(cpu_mask, path);
	free(path);

	return 0;
}

int
read_cpu_topology(struct cpu_topology *topo)
{
	struct cpu *cpu;
	unsigned long ncpus;
	size_t i;

	if (sysfs_parse_max_from_list(&ncpus, "/sys/devices/system/cpu/present") < 0)
		return -1;

	++ncpus;

	topo->cpus = calloc(ncpus, sizeof *topo->cpus);
	topo->ncpus = ncpus;

	for (i = 0; i < ncpus; ++i) {
		cpu = topo->cpus + i;

		if (bitmap_alloc(&cpu->core_siblings, ncpus) < 0)
			return -1;

		if (bitmap_alloc(&cpu->thread_siblings, ncpus) < 0)
			return -1;

		read_core_id(&cpu->core_id, i);
		read_package_id(&cpu->package_id, i);
		read_core_siblings(&cpu->core_siblings, i);
		read_thread_siblings(&cpu->thread_siblings, i);
	}

	return 0;
}
