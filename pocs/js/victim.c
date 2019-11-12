#include <string.h>
#include <stdio.h>
int main(int argc, char **argv) {
	char __attribute__((aligned(4096))) buffer[64*64];
	char val[32] = "Hello World! It's me Mario!";
	
	memset(buffer, 0x11, 64*64);
	asm volatile("vmovdqa (%0), %%ymm0"::"r"(val):"ymm0");
	asm volatile("nop\n"::"r"(buffer));
	while (1) {
		
		asm volatile("vmovntdq %%ymm0, 0(%0)"::"r"(buffer):"ymm0");
		asm volatile("mfence");

	}
	
	return -1;
}
