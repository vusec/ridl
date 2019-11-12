#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <sys/mman.h>
#include <unistd.h>

#define nop "nop\n"
#define nop5 nop nop nop nop nop
#define nop10 nop5 nop5
#define nop50 nop10 nop10 nop10 nop10 nop10
#define nop100 nop50 nop50
#define nop500 nop100 nop100 nop100 nop100 nop100

int main(int argc, char **argv) {
	unsigned char __attribute__((aligned(4096))) buffer[4096];

	while (1) {
		asm volatile (nop50);
#ifdef MFENCE_BEFORE
		asm volatile ("mfence");
#endif
		*(volatile unsigned char *)buffer = 0x84;
#ifdef MFENCE_AFTER
		asm volatile ("mfence");
#endif
	}
}
