#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <sys/mman.h>

int main(int argc, char **argv) {
	unsigned char __attribute__((aligned(64))) buffer[64];
	memset(buffer, 0x64, 64);
	unsigned char c = 4;
	while (1) {
		*(volatile unsigned char *)buffer = c;
		c += 0x40;
#ifdef MFENCE
		asm volatile ("mfence");
#endif
	}
}
