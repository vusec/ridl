#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <sys/mman.h>

int main(int argc, char **argv) {
	while (1) {
#ifdef NOP
		asm volatile ("nop");
#endif
#ifdef MFENCE
		asm volatile ("mfence");
#endif
	}
}
