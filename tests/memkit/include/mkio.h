#pragma once

/* /dev/memkit/kmem ioctls */
enum {
	MKIO_KMALLOC = 1,
	MKIO_KFREE,
	MKIO_VMALLOC,
	MKIO_VFREE,
	MKIO_READ,
	MKIO_WRITE,
	MKIO_TIMED_LOAD,
	MKIO_TIMED_STORE,
	MKIO_PREFETCHT0,
	MKIO_PREFETCHT1,
	MKIO_PREFETCHT2,
	MKIO_PREFETCHNTA,
	MKIO_CLFLUSH,
	MKIO_CLFLUSHOPT,
};

struct kmem_param {
	char *buf;
	void *addr;
	size_t size;
	uint64_t dt;
};

/* /dev/memkit/tlb ioctls */
enum {
	MKIO_FLUSH_ALL = 1,
	MKIO_FLUSH_PID,
	MKIO_FLUSH_RANGE,
	MKIO_FLUSH_PAGE,
	MKIO_RESOLVE,
	MKIO_UPDATE,
};

struct pte_walk {
	uint64_t pgd, p4d, pud, pmd, pte;
};

struct tlb_param {
	struct pte_walk walk;
	pid_t pid;
	void *base;
	size_t size;
};

