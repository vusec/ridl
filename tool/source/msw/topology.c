#include <stdlib.h>
#include <string.h>

#include <bitmap.h>
#include <macros.h>
#include <intrin.h>
#include <platform.h>

#include <info/topology.h>

size_t
count_mask(PROCESSOR_RELATIONSHIP *info)
{
	size_t count = 0;
	size_t i;

	for (i = 0; i < info->GroupCount; ++i) {
		count += sys_popcount64(info->GroupMask[i].Mask);
	}

	return count;
}

void
set_bit_mask(struct bitmap *bmap, PROCESSOR_RELATIONSHIP *info)
{
	size_t bits = BIT_SIZE(ULONG_PTR);
	ULONG_PTR mask;
	size_t i, j;

	for (j = 0; j < info->GroupCount; ++j) {
		for (i = 0; i < bits; ++i) {
			mask = ((ULONG_PTR)(1) << i);

			if (!(info->GroupMask[j].Mask & mask))
				continue;

			bitmap_set(bmap, j * bits + i);
		}
	}
}

void
set_core_siblings(struct cpu *cpus, PROCESSOR_RELATIONSHIP *info,
	unsigned package_id)
{
	struct cpu *cpu;
	size_t bits = BIT_SIZE(ULONG_PTR);
	ULONG_PTR mask;
	size_t i, j;

	for (j = 0; j < info->GroupCount; ++j) {
		for (i = 0; i < bits; ++i) {
			mask = ((ULONG_PTR)(1) << i);

			if (!(info->GroupMask[j].Mask & mask))
				continue;

			cpu = cpus + j * bits + i;
			cpu->package_id = package_id;
			set_bit_mask(&cpu->core_siblings, info);
		}
	}
}

void
set_thread_siblings(struct cpu *cpus, PROCESSOR_RELATIONSHIP *info,
	unsigned core_id)
{
	struct cpu *cpu;
	size_t bits = BIT_SIZE(ULONG_PTR);
	ULONG_PTR mask;
	size_t i, j;

	for (j = 0; j < info->GroupCount; ++j) {
		for (i = 0; i < bits; ++i) {
			mask = ((ULONG_PTR)(1) << i);

			if (!(info->GroupMask[j].Mask & mask))
				continue;

			cpu = cpus + j * bits + i;
			cpu->core_id = core_id;
			set_bit_mask(&cpu->thread_siblings, info);
		}
	}
}

struct cpu *
alloc_cpus(size_t ncpus)
{
	struct cpu *cpus, *cpu;
	size_t i;

	if (!ncpus)
		return NULL;

	cpus = calloc(ncpus, sizeof *cpus);

	for (i = 0; i < ncpus; ++i) {
		cpu = cpus + i;

		if (bitmap_alloc(&cpu->core_siblings, ncpus) < 0)
			return NULL;

		if (bitmap_alloc(&cpu->thread_siblings, ncpus) < 0)
			return NULL;
	}

	return cpus;
}

int
read_cpu_topology(struct cpu_topology *topo)
{
	PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX info = NULL;
	char *buffer = NULL;
	char *p;
	unsigned long ncpus = 0;
	unsigned long core_id = 0;
	unsigned long package_id = 0;
	DWORD len = 0;

	GetLogicalProcessorInformationEx(RelationAll,
		(PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX)buffer, &len);

	if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
		return -1;

	buffer = malloc(len);

	if (!buffer)
		return -1;

	if (!GetLogicalProcessorInformationEx(RelationAll,
		(PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX)buffer, &len))
		goto err_free;

	for (p = buffer; p < buffer + len; p += info->Size) {
		info = (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX)p;

		if (info->Relationship != RelationProcessorCore)
			continue;

		ncpus += count_mask(&info->Processor);
	}

	if (!ncpus)
		goto err_free;

	topo->cpus = alloc_cpus(ncpus);
	topo->ncpus = ncpus;

	if (!topo->cpus)
		goto err_free;

	for (p = buffer; p < buffer + len; p += info->Size) {
		info = (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX)p;

		switch (info->Relationship) {
		case RelationProcessorPackage:
			set_core_siblings(topo->cpus, &info->Processor,
				package_id);
			++package_id;
			break;
		case RelationProcessorCore:
			set_thread_siblings(topo->cpus, &info->Processor,
				core_id);
			++core_id;
			break;
		default: break;
		}
	}

	free(buffer);

	return 0;

err_free:
	free(buffer);
	return -1;
}
