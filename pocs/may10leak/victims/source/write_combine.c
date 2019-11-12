#include "boilerplate.h"

int main() {
	BOILERPLATE_INIT();

	asm volatile ("andq $0xffffffffffff0000, %rsp\n");
	asm volatile ("movl $0x0, -64(%rsp)\n");
	while (1) {
		asm volatile(
			"movl $0x12345678, -60(%rsp)\n"
			"mfence\n"
		);
	}
}
