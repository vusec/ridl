int main(void)
{
	/* Leak 'N' by reading from offset 1. */
	volatile char source[32] = "XNXXYYYYZZZZQQQQAAAABBBBCCCCDDDD";

	/* Victim code runs in a loop. */
	while (1) {
		/* Move value into the load part. */
		asm volatile(
			"movdqu 1(%0), %%xmm0\n"
			:: "r"(&source) : "rax");
	}
}

