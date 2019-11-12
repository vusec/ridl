#pragma once

#include <x86intrin.h>

#define force_inline __attribute__(force_inline)

struct hit_count {
	unsigned char index;
	size_t count;
};

int init_sigint(void);
void enable_ac(void);
void disable_ac(void);
void retpol_probe(void *buffer, void *from);
void tsx_probe(void *buffer, void *from);
void retpol_probe2(void *buffer, void *from);
void tsx_probe2(void *buffer, void *from);
void tsx_probe_shift(void *buffer, void *from, size_t offset);
void avx_probe(void *buffer, void *from);
void sse_probe(void *buffer, void *from);
void probe(void *buffer, void *from);
void clear_results(struct hit_count *results);
void time_buffer(struct hit_count *results, unsigned char *buffer, uint64_t threshold);
int cmp_results(const void *lhs_, const void *rhs_);
int match_best(unsigned char *byte, size_t *count, struct hit_count *results);
void print_hexstring(const unsigned char *s, size_t len);
void print_string(const unsigned char *s, size_t len);
void print_result(const unsigned char *s, size_t len, void *src);

