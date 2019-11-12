#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <sys/mman.h>

#ifndef ITERS
#define ITERS 200000
#endif

int main(int argc, char **argv) {
	volatile char *buffer = mmap((void *)0x4ef00000, 4096, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE | MAP_FIXED, -1, 0);
	char *src =  "#00#ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz12345678";
	char buf[64];
	memcpy(buf, src, 64);
	while (1) {
		for (int i = 0; i < ITERS; ++i) {
			memcpy(buffer, buf, 64);
		}
		if (buf[2] == '9') {
			if (buf[1] == '9')
				buf[1] = '0';
			else
				buf[1]++;
			buf[2] = '0';
		} else
			buf[2]++;
	}
}
