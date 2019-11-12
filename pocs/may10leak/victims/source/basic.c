#include "boilerplate.h"

#ifndef SECRET
#define SECRET 0xa1a2a3a4
#endif

#ifndef OFFSET
#define OFFSET 0
#endif

int main() {
	BOILERPLATE_INIT();

	asm volatile (
		"andq $0xffffffffffff0000, %rsp\n"
#ifdef READ
#ifdef SHORT_SECRET
		"movq $" TOSTRING(SECRET) ", " TOSTRING(OFFSET) "(%rsp)\n"
#else
		"movq $" TOSTRING(SECRET) ", %rax\n"
		"movq %rax, " TOSTRING(OFFSET) "(%rsp)\n"
#endif
#else // READ
#ifndef SHORT_SECRET
		"movq $" TOSTRING(SECRET) ", %rax\n"
#endif
#endif // READ
	);
	while (1) {
		asm volatile(
#ifdef READ
			"movq " TOSTRING(OFFSET) "(%rsp), %rax\n"
#ifdef CLEAR_AFTER_READ
			"xorq %rax, %rax\n"
#endif
#else // READ
#ifdef SHORT_SECRET
			"movq $" TOSTRING(SECRET) ", " TOSTRING(OFFSET) "(%rsp)\n"
#else
	#ifdef USE_NT
			"movnti %rax, " TOSTRING(OFFSET) "(%rsp)\n"
	#else
			"movq %rax, " TOSTRING(OFFSET) "(%rsp)\n"
	#endif
#endif
#endif // READ
#ifdef MFENCE
			"mfence\n"
#endif
		);
	}
}
