	pid_t pid = fork();
	if (pid == 0) {
		char *buffer = memalign(4096, 1024);
		asm volatile(
			"movq %0, %%xmm0\n"
			"punpcklbw %%xmm0, %%xmm0\n"
			"punpcklwd %%xmm0, %%xmm0\n"
			"pshufd $0x0, %%xmm0, %%xmm0\n"
			::"r"((unsigned long long)SECRET_VALUE)
		);
		while (1) {
			asm volatile(
				"movdqa %%xmm0, 0(%0)\n"
				::"r"(buffer):"xmm0"
			);
		}
	}
	if (pid < 0) return 1;

