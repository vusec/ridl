#include <stdio.h>
#include <stdint.h>
#include <unistd.h>

int main() {
	usleep(10000);
	while (1) {
		unsigned long long randnum;
		unsigned char *randptr = (unsigned char *)&randnum;
		asm volatile("rdrand %%rax\n":"=a"(randnum));
		// flip bytes so they match the leak output nicely
		printf("rdrand: random number: %02x%02x%02x%02x\n", randptr[0], randptr[1], randptr[2], randptr[3]);
		usleep(500000);
	}
}
